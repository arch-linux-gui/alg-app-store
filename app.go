package main

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"log"
	"os"
	"sort"
	"strings"
	"sync"
	"time"

	"os/exec"

	"github.com/Jguer/go-alpm/v2"
	paconf "github.com/Morganamilo/go-pacmanconf"
)

var h *alpm.Handle

// var dbs []alpm.IDB
var dbs []alpm.IDB

var DesktopEnv string

// App struct
type App struct {
	ctx context.Context
}

// NewApp creates a new App application struct
func NewApp() *App {
	return &App{}
}

// startup is called at application startup
func (a *App) startup(ctx context.Context) {
	// Perform your setup here
	a.ctx = ctx

	DesktopEnv = getDesktopEnvironment()

	var err error
	h, err = alpm.Initialize("/", "/var/lib/pacman")
	if err != nil {
		fmt.Fprintf(os.Stderr, "failed to initialize alpm: %v\n", err)
		os.Exit(1)
	}

	// Register and sync repositories
	repos := []string{"core", "extra"}
	for _, repo := range repos {
		db, err := h.RegisterSyncDB(repo, 0)
		if err != nil {
			fmt.Printf("Error getting sync db for %s: %v\n", repo, err)
			return
		}
		dbs = append(dbs, db)
	}
}

// domReady is called after front-end resources have been loaded
func (a App) domReady(ctx context.Context) {
	// Add your action here
}

// beforeClose is called when the application is about to quit,
// either by clicking the window close button or calling runtime.Quit.
// Returning true will cause the application to continue, false will continue shutdown as normal.
func (a *App) beforeClose(ctx context.Context) (prevent bool) {
	return false
}

// shutdown is called at application termination
func (a *App) shutdown(ctx context.Context) {
	// Perform your teardown here
	if h != nil {
		h.Release()
	}
}

type PackageInfo struct {
	Name        string   `json:"name"`
	Version     string   `json:"version"`
	Description string   `json:"description"`
	Repository  string   `json:"repository"`
	Maintainer  string   `json:"maintainer"`
	UpstreamURL string   `json:"upstreamurl"`
	DependList  []string `json:"dependlist"`
	LastUpdated string   `json:"lastupdated"`
}

func (a *App) SearchPackage(query string) []PackageInfo {
	var results []PackageInfo
	var wg sync.WaitGroup
	resultChan := make(chan PackageInfo, 100)
	doneChan := make(chan bool)

	// Start a goroutine to collect results
	go func() {
		for pkg := range resultChan {
			results = append(results, pkg)
		}
		doneChan <- true
	}()

	// Search official repositories concurrently
	for _, db := range dbs {
		wg.Add(1)
		go func(db alpm.IDB) {
			defer wg.Done()
			searchDB(db, query, resultChan)
		}(db)
	}

	// Search AUR concurrently
	wg.Add(1)
	go func() {
		defer wg.Done()
		searchAUR(query, resultChan)
	}()

	// Wait for all searches to complete
	wg.Wait()
	close(resultChan)

	// Wait for result collection to finish
	<-doneChan

	return results
}

func searchDB(db alpm.IDB, query string, resultChan chan<- PackageInfo) {
	db.PkgCache().ForEach(func(pkg alpm.IPackage) error {
		if strings.Contains(strings.ToLower(pkg.Name()), strings.ToLower(query)) {
			lastUpdated := pkg.BuildDate().UTC().Format("Jan. 2, 2006, 3 p.m. MST")
			resultChan <- PackageInfo{
				Name:        pkg.Name(),
				Version:     pkg.Version(),
				Description: pkg.Description(),
				Repository:  db.Name(),
				Maintainer:  pkg.Packager(),
				UpstreamURL: pkg.URL(),
				DependList:  convertDependList(pkg.Depends()),
				LastUpdated: lastUpdated,
			}
		}
		return nil
	})
}

func searchAUR(query string, resultChan chan<- PackageInfo) {
	cmd := exec.Command("curl", "-s", fmt.Sprintf("https://aur.archlinux.org/rpc/?v=5&type=search&arg=%s", query))
	output, err := cmd.Output()
	if err != nil {
		fmt.Printf("Error searching AUR: %v\n", err)
		return
	}

	var aurResponse struct {
		Results []struct {
			Name         string `json:"Name"`
			Version      string `json:"Version"`
			Description  string `json:"Description"`
			Maintainer   string `json:"Maintainer"`
			URL          string `json:"URL"`
			LastModified int64  `json:"LastModified"`
		} `json:"results"`
	}
	err = json.Unmarshal(output, &aurResponse)
	if err != nil {
		fmt.Printf("Error parsing AUR response: %v\n", err)
		return
	}

	for _, aurPkg := range aurResponse.Results {
		lastUpdated := time.Unix(aurPkg.LastModified, 0).UTC().Format("02-01-2006")
		resultChan <- PackageInfo{
			Name:        aurPkg.Name,
			Version:     aurPkg.Version,
			Description: aurPkg.Description,
			Repository:  "AUR",
			Maintainer:  aurPkg.Maintainer,
			UpstreamURL: aurPkg.URL,
			DependList:  nil,
			LastUpdated: lastUpdated,
		}
	}
}

func convertDependList(depList alpm.IDependList) []string {
	var deps []string
	depList.ForEach(func(dep *alpm.Depend) error {
		deps = append(deps, dep.Name)
		return nil
	})
	return deps
}

func (a *App) GetInstalledPackages() ([]PackageInfo, error) {
	h, err := alpm.Initialize("/", "/var/lib/pacman")
	if err != nil {
		fmt.Fprintf(os.Stderr, "failed to initialize alpm: %v\n", err)
		os.Exit(1)
	}

	if h == nil {
		return nil, fmt.Errorf("ALPM handle is not initialized")
	}

	db, err := h.LocalDB()
	if err != nil {
		return nil, fmt.Errorf("failed to get local DB: %v", err)
	}

	var packages []PackageInfo
	var mutex sync.Mutex

	err = db.PkgCache().ForEach(func(pkg alpm.IPackage) error {
		mutex.Lock()
		lastUpdated := pkg.BuildDate().UTC().Format("Jan. 2, 2006, 3 p.m. MST")
		packages = append(packages, PackageInfo{
			Name:        pkg.Name(),
			Version:     pkg.Version(),
			Description: pkg.Description(),
			Repository:  pkg.DB().Name(),
			Maintainer:  pkg.Packager(),
			UpstreamURL: pkg.URL(),
			DependList:  convertDependList(pkg.Depends()),
			LastUpdated: lastUpdated,
		})
		mutex.Unlock()
		return nil
	})

	if err != nil {
		return nil, fmt.Errorf("error iterating over packages: %v", err)
	}

	if len(packages) == 0 {
		return nil, fmt.Errorf("no installed packages found")
	}

	return packages, nil
}

func (a *App) SearchLocalPackage(pkg string) (bool, error) {
	if pkg == "" {
		return false, fmt.Errorf("empty package name provided")
	}

	local, err := searchLocalDB(pkg)
	if err != nil {
		return false, fmt.Errorf("error searching local DB: %w", err)
	}

	if local == nil {
		return false, nil
	}

	fmt.Println(strings.Contains(strings.ToLower(local.Name()), strings.ToLower(pkg)))

	return strings.Contains(strings.ToLower(local.Name()), strings.ToLower(pkg)), nil
}

func searchLocalDB(pkg string) (alpm.IPackage, error) {
	h, err := alpm.Initialize("/", "/var/lib/pacman")
	if err != nil {
		fmt.Fprintf(os.Stderr, "failed to initialize alpm: %v\n", err)
		os.Exit(1)
	}
	if h == nil {
		return nil, fmt.Errorf("ALPM handle is not initialized")
	}

	db, err := h.LocalDB()
	if err != nil {
		return nil, fmt.Errorf("failed to get local DB: %w", err)
	}

	res := db.Pkg(pkg)
	return res, nil
}

func (a *App) CheckPackageInstalled(packageName string) bool {
	h, err := alpm.Initialize("/", "/var/lib/pacman")
	if err != nil {
		fmt.Fprintf(os.Stderr, "failed to initialize alpm: %v\n", err)
		os.Exit(1)
	}
	if h == nil {
		log.Fatal("ALPM handle is not initialized")
	}
	localDB, err := h.LocalDB()
	if err != nil {
		log.Fatal(err)
	}
	packageHandle := localDB.Pkg(packageName)
	return packageHandle != nil
}

func (a *App) Install(pkg string) {
	cmdStr := fmt.Sprintf("pkexec yay -S %s --noconfirm", pkg)
	cmd := exec.Command("sh", "-c", cmdStr)
	fmt.Println("Executing command:", cmdStr)

	var outBuffer, errBuffer bytes.Buffer
	cmd.Stdout = &outBuffer
	cmd.Stderr = &errBuffer

	err := cmd.Run()
	if err != nil {
		fmt.Println("Error executing command:", err)
		fmt.Println("stderr:", errBuffer.String())
		return
	}

	fmt.Println("stdout:", outBuffer.String())
	fmt.Println("stderr:", errBuffer.String())
}

func (a *App) Uninstall(pkg string) {
	cmdStr := fmt.Sprintf("pkexec yay -Rdd %s --noconfirm", pkg)
	cmd := exec.Command("sh", "-c", cmdStr)
	fmt.Println("Executing command:", cmdStr)

	var outBuffer, errBuffer bytes.Buffer
	cmd.Stdout = &outBuffer
	cmd.Stderr = &errBuffer

	err := cmd.Run()
	if err != nil {
		fmt.Println("Error executing command:", err)
		fmt.Println("stderr:", errBuffer.String())
		return
	}

	fmt.Println("stdout:", outBuffer.String())
	fmt.Println("stderr:", errBuffer.String())
}

// func openTerminal(cmd string) {
// 	var pkexecCmd *exec.Cmd

// 	switch DesktopEnv {
// 	case "xfce":
// 		pkexecCmd = exec.Command("xfce4-terminal", "-e", cmd)
// 	case "gnome":
// 		pkexecCmd = exec.Command("gnome-terminal", "--", "bash", "-c", cmd)
// 	case "kde":
// 		pkexecCmd = exec.Command("konsole", "-e", cmd)
// 	case "mate":
// 		pkexecCmd = exec.Command("mate-terminal", "-e", cmd)
// 	case "lxde":
// 		pkexecCmd = exec.Command("lxterminal", "-e", cmd)
// 	case "lxqt":
// 		pkexecCmd = exec.Command("qterminal", "-e", cmd)
// 	default:
// 		fmt.Printf("Unsupported desktop environment: %s\n", DesktopEnv)
// 		return
// 	}

// 	if err := pkexecCmd.Run(); err != nil {
// 		fmt.Printf("Error executing command: %v\n", err)
// 	}
// }

func getDesktopEnvironment() string {
	return strings.ToLower(os.Getenv("XDG_CURRENT_DESKTOP"))
}

func (a *App) GetMultiplePackageInfo(packageNames []string) ([]PackageInfo, error) {
	var results []PackageInfo
	var wg sync.WaitGroup
	resultChan := make(chan PackageInfo, len(packageNames))
	errorChan := make(chan error, len(packageNames))

	for _, pkgName := range packageNames {
		wg.Add(1)
		go func(name string) {
			defer wg.Done()

			// Search for package info
			searchResults := a.SearchPackage(name)

			var pkg PackageInfo
			for _, result := range searchResults {
				if result.Repository == "core" || result.Repository == "extra" || result.Repository == "AUR" {
					pkg = result
					pkg.Name = name // Ensure the name matches the search query
					resultChan <- pkg
					return
				}
			}

			// If no package was found in core, extra, or AUR, add a placeholder
			if pkg.Name == "" {
				resultChan <- PackageInfo{
					Name:        name,
					Description: "Package not found in core, extra, or AUR",
					Repository:  "unknown",
				}
			}
		}(pkgName)
	}

	// Close channels when all goroutines are done
	go func() {
		wg.Wait()
		close(resultChan)
		close(errorChan)
	}()

	// Collect results and errors
	for i := 0; i < len(packageNames); i++ {
		select {
		case result := <-resultChan:
			results = append(results, result)
		case err := <-errorChan:
			return nil, fmt.Errorf("error processing packages: %w", err)
		case <-a.ctx.Done():
			return nil, a.ctx.Err()
		}
	}

	// Sort results to maintain order of input packageNames
	sort.Slice(results, func(i, j int) bool {
		iIndex := indexOf(packageNames, results[i].Name)
		jIndex := indexOf(packageNames, results[j].Name)
		return iIndex < jIndex
	})

	return results, nil
}

func indexOf(slice []string, item string) int {
	for i, s := range slice {
		if s == item {
			return i
		}
	}
	return -1
}

type UpdateInfo struct {
	Name         string `json:"name"`
	OldVersion   string `json:"oldVersion"`
	NewVersion   string `json:"newVersion"`
	Repository   string `json:"repository"`
	DownloadSize int64  `json:"downloadSize"`
}

// GetAvailableUpdates returns a list of available updates for packages
func (a *App) GetAvailableUpdates() ([]UpdateInfo, error) {
	// Initialize ALPM
	h, err := alpm.Initialize("/", "/var/lib/pacman")
	if err != nil {
		return nil, fmt.Errorf("failed to initialize alpm: %v", err)
	}
	defer h.Release()

	// Parse pacman configuration
	pacmanConfig, _, err := paconf.ParseFile("/etc/pacman.conf")
	if err != nil {
		return nil, fmt.Errorf("failed to parse pacman config: %v", err)
	}

	// Register sync databases
	for _, repo := range pacmanConfig.Repos {
		db, err := h.RegisterSyncDB(repo.Name, 0)
		if err != nil {
			return nil, fmt.Errorf("failed to register sync db %s: %v", repo.Name, err)
		}
		db.SetServers(repo.Servers)
	}

	// Get local database
	localDB, err := h.LocalDB()
	if err != nil {
		return nil, fmt.Errorf("failed to get local DB: %v", err)
	}

	// Get sync databases
	syncDBs, err := h.SyncDBs()
	if err != nil {
		return nil, fmt.Errorf("failed to get sync DBs: %v", err)
	}

	var updates []UpdateInfo
	var mutex sync.Mutex
	var wg sync.WaitGroup

	// Check for updates in official repositories
	wg.Add(1)
	go func() {
		defer wg.Done()
		for _, pkg := range localDB.PkgCache().Slice() {
			select {
			case <-a.ctx.Done():
				return
			default:
				newPkg := pkg.SyncNewVersion(syncDBs)
				if newPkg != nil {
					mutex.Lock()
					updates = append(updates, UpdateInfo{
						Name:         pkg.Name(),
						OldVersion:   pkg.Version(),
						NewVersion:   newPkg.Version(),
						Repository:   newPkg.DB().Name(),
						DownloadSize: newPkg.Size(),
					})
					mutex.Unlock()
				}
			}
		}
	}()

	// Check for AUR updates
	wg.Add(1)
	go func() {
		defer wg.Done()
		aurUpdates, err := a.checkAURUpdates()
		if err != nil {
			log.Printf("Error checking AUR updates: %v", err)
			return
		}
		mutex.Lock()
		updates = append(updates, aurUpdates...)
		mutex.Unlock()
	}()

	wg.Wait()

	return updates, nil
}

func (a *App) checkAURUpdates() ([]UpdateInfo, error) {
	var aurUpdates []UpdateInfo

	// Get list of AUR packages
	cmd := exec.CommandContext(a.ctx, "yay", "-Qm")
	output, err := cmd.Output()
	if err != nil {
		return nil, fmt.Errorf("failed to get AUR package list: %v", err)
	}

	aurPackages := strings.Split(strings.TrimSpace(string(output)), "\n")

	for _, pkg := range aurPackages {
		select {
		case <-a.ctx.Done():
			return aurUpdates, a.ctx.Err()
		default:
			parts := strings.Fields(pkg)
			if len(parts) != 2 {
				continue
			}
			name, version := parts[0], parts[1]

			// Check for updates using AUR RPC
			cmd := exec.CommandContext(a.ctx, "curl", "-s", fmt.Sprintf("https://aur.archlinux.org/rpc/v5/info/%s", name))
			output, err := cmd.Output()
			if err != nil {
				log.Printf("Error checking AUR for package %s: %v", name, err)
				continue
			}

			var aurResponse struct {
				Results []struct {
					Version string `json:"Version"`
				} `json:"results"`
			}
			err = json.Unmarshal(output, &aurResponse)
			if err != nil {
				log.Printf("Error parsing AUR response for package %s: %v", name, err)
				continue
			}

			if len(aurResponse.Results) > 0 && aurResponse.Results[0].Version != version {
				aurUpdates = append(aurUpdates, UpdateInfo{
					Name:         name,
					OldVersion:   version,
					NewVersion:   aurResponse.Results[0].Version,
					Repository:   "AUR",
					DownloadSize: 0,
				})
			}
		}
	}

	return aurUpdates, nil
}

// HumanReadableSize converts bytes to a human-readable string
func (a *App) HumanReadableSize(size int64) string {
	floatsize := float32(size)
	units := [...]string{"", "Ki", "Mi", "Gi", "Ti", "Pi", "Ei", "Zi", "Yi"}
	for _, unit := range units {
		if floatsize < 1024 {
			return fmt.Sprintf("%.1f %sB", floatsize, unit)
		}
		floatsize /= 1024
	}
	return fmt.Sprintf("%d%s", size, "B")
}

func (a *App) UpdateSinglePkg(pkg string) {
	cmdStr := fmt.Sprintf("pkexec yay -S %s --noconfirm", pkg)
	cmd := exec.Command("sh", "-c", cmdStr)
	fmt.Println("Executing command:", cmdStr)

	var outBuffer, errBuffer bytes.Buffer
	cmd.Stdout = &outBuffer
	cmd.Stderr = &errBuffer

	err := cmd.Run()
	if err != nil {
		fmt.Println("Error executing command:", err)
		fmt.Println("stderr:", errBuffer.String())
		return
	}

	fmt.Println("stdout:", outBuffer.String())
	fmt.Println("stderr:", errBuffer.String())
}

func (a *App) UpdateAllPkg() {
	cmdStr := "pkexec yay -Syu --noconfirm"
	cmd := exec.Command("sh", "-c", cmdStr)
	fmt.Println("Executing command:", cmdStr)

	var outBuffer, errBuffer bytes.Buffer
	cmd.Stdout = &outBuffer
	cmd.Stderr = &errBuffer

	err := cmd.Run()
	if err != nil {
		fmt.Println("Error executing command:", err)
		fmt.Println("stderr:", errBuffer.String())
		return
	}

	fmt.Println("stdout:", outBuffer.String())
	fmt.Println("stderr:", errBuffer.String())
}
