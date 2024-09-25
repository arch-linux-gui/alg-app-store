package getpackage

import (
	"encoding/json"
	"fmt"
	"os"
	"os/exec"
	"strings"
	"sync"
	"time"

	"github.com/Jguer/go-alpm/v2"
)

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

func SavePackageInfoToFile(pkgName, fileName string) error {
	pkgs := SearchPackage(pkgName)
	if len(pkgs) == 0 {
		return fmt.Errorf("package %s not found", pkgName)
	}

	// Assume the first result is the most relevant one
	pkgInfo := pkgs

	file, err := os.Create(fileName)
	if err != nil {
		return fmt.Errorf("failed to create file: %w", err)
	}
	defer file.Close()

	encoder := json.NewEncoder(file)
	encoder.SetIndent("", "  ")
	err = encoder.Encode(pkgInfo)
	if err != nil {
		return fmt.Errorf("failed to encode package info to JSON: %w", err)
	}

	fmt.Printf("Package information for %s saved to %s\n", pkgName, fileName)
	return nil
}

func SearchPackage(query string) []PackageInfo {
	var h *alpm.Handle

	// var dbs []alpm.IDB
	var dbs []alpm.IDB
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
		}
		dbs = append(dbs, db)
	}
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

func main() {
	err := SavePackageInfoToFile("google-chrome", "package-info.json")
	if err != nil {
		fmt.Println("Error:", err)
	}
}
