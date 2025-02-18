import { useCallback, useEffect, useState } from "react";
import {
  Card,
  CardContent,
  CardDescription,
  CardFooter,
  CardHeader,
  CardTitle,
} from "../components/ui/card";
import { Badge } from "../components/ui/badge";
import { ScrollArea } from "../components/ui/scroll-area";
import PackageDetails from "./PackageDetails";
import ErrorBoundary from "./ErrorBoundary";
import { CheckPackageInstalled } from "../../wailsjs/go/main/App";

interface AppInfo {
  name: string;
  version: string;
  description: string;
  repository: string;
  maintainer: string;
  upstreamurl: string;
  dependlist: string[];
  lastupdated: string;
}

const featuredApps: AppInfo[] = [
  {
    name: "firefox",
    version: "128.0.2-1",
    description: "Fast, Private \u0026 Safe Web Browser",
    repository: "extra",
    maintainer:
      "Jan Alexander Steffens (heftig) \u003cheftig@archlinux.org\u003e",
    upstreamurl: "https://www.mozilla.org/firefox/",
    dependlist: [
      "alsa-lib",
      "at-spi2-core",
      "bash",
      "cairo",
      "dbus",
      "ffmpeg",
      "fontconfig",
      "freetype2",
      "gcc-libs",
      "gdk-pixbuf2",
      "glib2",
      "glibc",
      "gtk3",
      "hicolor-icon-theme",
      "libpulse",
      "libx11",
      "libxcb",
      "libxcomposite",
      "libxdamage",
      "libxext",
      "libxfixes",
      "libxrandr",
      "libxss",
      "libxt",
      "mime-types",
      "nspr",
      "nss",
      "pango",
      "ttf-font",
    ],
    lastupdated: "Jul. 23, 2024, 4 p.m. UTC",
  },
  {
    name: "gimp",
    description: "GNU Image Manipulation Program",
    repository: "extra",
    version: "2.10.38-1",
    maintainer: "Christian Hesse, Christian Heusel",
    lastupdated: "May 3, 2024, 9:48 a.m. UTC",
    dependlist: [
      "aalib",
      "babl",
      "bzip2",
      "cairo",
      "fontconfig",
      "freetype2",
      "gcc-libs",
      "gdk-pixbuf2",
      "gegl",
      "glib2",
      "glibc",
      "gtk2",
      "harfbuzz",
      "hicolor-icon-theme",
      "iso-codes",
      "json-glib",
      "lcms2",
      "libgexiv2",
      "libgudev",
      "libheif",
      "libjpeg-turbo",
      "libjxl",
      "libmng",
      "libmypaint",
      "libpng",
      "librsvg",
      "libtiff",
      "libunwind",
      "libwebp",
      "libwmf",
      "libx11",
      "libxcursor",
      "libxext",
      "libxfixes",
      "libxmu",
      "libxpm",
      "mypaint-brushes1",
      "openexr",
      "openjpeg2",
      "pango",
      "poppler-data",
      "poppler-glib",
      "xz",
      "zlib",
    ],
    upstreamurl: "https://www.gimp.org/",
  },
  {
    name: "vlc",
    version: "3.0.21-1",
    description: "Multi-platform MPEG, VCD/DVD, and DivX player",
    repository: "extra",
    maintainer: "Antonio Rojas \u003carojas@archlinux.org\u003e",
    upstreamurl: "https://www.videolan.org/vlc/",
    dependlist: [
      "a52dec",
      "abseil-cpp",
      "aribb24",
      "bash",
      "cairo",
      "dbus",
      "faad2",
      "ffmpeg4.4",
      "fontconfig",
      "freetype2",
      "fribidi",
      "gcc-libs",
      "gdk-pixbuf2",
      "glib2",
      "glibc",
      "gnutls",
      "harfbuzz",
      "hicolor-icon-theme",
      "libarchive",
      "libdca",
      "libdvbpsi",
      "libglvnd",
      "libidn",
      "libmad",
      "libmatroska",
      "libmpcdec",
      "libmpeg2",
      "libproxy",
      "libsecret",
      "libtar",
      "libupnp",
      "libixml.so",
      "libupnp.so",
      "libva",
      "libx11",
      "libxcb",
      "libxinerama",
      "libxml2",
      "libxpm",
      "lua",
      "qt5-base",
      "qt5-svg",
      "qt5-x11extras",
      "taglib",
      "wayland",
      "xcb-util-keysyms",
      "zlib",
    ],
    lastupdated: "Jun. 16, 2024, 8 p.m. UTC",
  },
  {
    name: "visual-studio-code-bin",
    description:
      "Visual Studio Code (vscode): Editor for building and debugging modern web and cloud applications (official binary version)",
    repository: "AUR",
    version: "1.57.0",
    maintainer: "Microsoft",
    lastupdated: "2023-06-10",
    dependlist: [
      "libxkbfile",
      "gnupg",
      "gtk3",
      "libsecret",
      "nss",
      "gcc-libs",
      "libnotify",
      "libxss",
      "glibc",
      "lsof",
      "shared-mime-info",
      "xdg-utils",
      "alsa-lib",
    ],
    upstreamurl: "",
  },
  {
    name: "libreoffice-still",
    version: "7.6.7-1",
    description: "LibreOffice maintenance branch",
    repository: "extra",
    maintainer: "Andreas Radke \u003candyrtr@archlinux.org\u003e",
    upstreamurl: "https://www.libreoffice.org/",
    dependlist: [
      "curl",
      "hunspell",
      "python",
      "libwpd",
      "libwps",
      "neon",
      "pango",
      "nspr",
      "libjpeg",
      "libxrandr",
      "libgl",
      "redland",
      "hyphen",
      "lpsolve",
      "gcc-libs",
      "sh",
      "graphite",
      "icu",
      "libxslt",
      "lcms2",
      "libvisio",
      "libetonyek",
      "libodfgen",
      "libcdr",
      "libmspub",
      "harfbuzz-icu",
      "nss",
      "clucene",
      "hicolor-icon-theme",
      "desktop-file-utils",
      "shared-mime-info",
      "libpagemaker",
      "libxinerama",
      "libabw",
      "libmwaw",
      "libe-book",
      "libcups",
      "liblangtag",
      "libexttextcat",
      "liborcus",
      "libwebp",
      "libcmis",
      "libtommath",
      "libzmf",
      "libatomic_ops",
      "libnumbertext",
      "gpgme",
      "libfreehand",
      "libstaroffice",
      "libepubgen",
      "libqxp",
      "libepoxy",
      "box2d",
      "zxing-cpp",
      "xdg-utils",
      "libldap",
      "fontconfig",
      "zlib",
      "libpng",
      "freetype2",
      "raptor",
      "libxml2",
      "cairo",
      "libx11",
      "expat",
      "glib2",
      "boost-libs",
      "libtiff",
      "dbus",
      "glibc",
      "librevenge",
      "libxext",
      "openjpeg2",
    ],
    lastupdated: "May. 19, 2024, 7 p.m. UTC",
  },
  {
    name: "blender",
    version: "17:4.2.0-3",
    description: "A fully integrated 3D graphics creation suite",
    repository: "extra",
    maintainer: "Sven-Hendrik Haase \u003csvenstaro@archlinux.org\u003e",
    upstreamurl: "https://www.blender.org",
    dependlist: [
      "alembic",
      "bash",
      "boost-libs",
      "draco",
      "embree",
      "expat",
      "ffmpeg",
      "fftw",
      "freetype2",
      "gcc-libs",
      "glew",
      "glibc",
      "gmp",
      "hicolor-icon-theme",
      "imath",
      "intel-oneapi-compiler-dpcpp-cpp-runtime-libs",
      "intel-oneapi-compiler-shared-runtime-libs",
      "jack",
      "jemalloc",
      "level-zero-loader",
      "libepoxy",
      "libharu",
      "libjpeg-turbo",
      "libpng",
      "libsndfile",
      "libspnav",
      "libtiff",
      "libwebp",
      "libx11",
      "libxfixes",
      "libxi",
      "libxkbcommon",
      "libxml2",
      "libxrender",
      "libxxf86vm",
      "llvm-libs",
      "materialx",
      "onetbb",
      "openal",
      "opencollada",
      "opencolorio",
      "openexr",
      "openimagedenoise",
      "openimageio",
      "openjpeg2",
      "openpgl",
      "openshadinglanguage",
      "opensubdiv",
      "openvdb",
      "openxr",
      "potrace",
      "pugixml",
      "pystring",
      "python",
      "python-numpy",
      "python-requests",
      "sdl2",
      "shared-mime-info",
      "usd",
      "xdg-utils",
      "yaml-cpp",
      "zlib",
      "zstd",
    ],
    lastupdated: "Jul. 17, 2024, 12 p.m. UTC",
  },
  {
    name: "zed",
    version: "0.144.4-1",
    description:
      "A high-performance, multiplayer code editor from the creators of Atom and Tree-sitter",
    repository: "extra",
    maintainer: "Caleb Maclennan \u003calerque@archlinux.org\u003e",
    upstreamurl: "https://zed.dev",
    dependlist: [
      "alsa-lib",
      "libasound.so",
      "fontconfig",
      "gcc-libs",
      "glibc",
      "libxcb",
      "libxkbcommon",
      "libxkbcommon-x11",
      "openssl",
      "libcrypto.so",
      "libssl.so",
      "sqlite",
      "vulkan-driver",
      "vulkan-icd-loader",
      "vulkan-tools",
      "wayland",
      "zlib",
      "libz.so",
    ],
    lastupdated: "Jul. 19, 2024, 9 p.m. UTC",
  },
  {
    name: "git",
    version: "2.45.2-1",
    description: "the fast distributed version control system",
    repository: "extra",
    maintainer: "Christian Hesse \u003ceworm@archlinux.org\u003e",
    upstreamurl: "https://git-scm.com/",
    dependlist: [
      "curl",
      "expat",
      "perl",
      "perl-error",
      "perl-mailtools",
      "openssl",
      "pcre2",
      "grep",
      "shadow",
      "zlib",
    ],
    lastupdated: "Jun. 1, 2024, 9 p.m. UTC",
  },
  {
    name: "google-chrome",
    version: "127.0.6533.72-1",
    description: "The popular web browser by Google (Stable Channel)",
    repository: "AUR",
    maintainer: "gromit",
    upstreamurl: "https://www.google.com/chrome",
    dependlist: [
      "alsa-lib",
      "gtk3",
      "libcups",
      "libxss",
      "libxtst",
      "nss",
      "ttf-liberation",
      "xdg-utils",
    ],
    lastupdated: "23-07-2024",
  },
];

const Home = () => {
  const [selectedApp, setSelectedApp] = useState<AppInfo | null>(null);
  const [installedApps, setInstalledApps] = useState<Set<string>>(new Set());

  const checkInstalledApps = useCallback(async () => {
    try {
      const installedSet = new Set<string>();
      for (const app of featuredApps) {
        try {
          const isInstalled = await CheckPackageInstalled(app.name);
          if (isInstalled) {
            installedSet.add(app.name);
          }
        } catch (error) {
          console.error(`Error checking if ${app.name} is installed:`, error);
        }
      }
      setInstalledApps(installedSet);
    } catch (error) {
      console.error("Error fetching package information:", error);
    }
  }, []);

  useEffect(() => {
    checkInstalledApps();
    const interval = setInterval(checkInstalledApps, 3000);
    return () => clearInterval(interval);
  }, [checkInstalledApps]);

  const handleInstallStateChange = useCallback(() => {
    checkInstalledApps();
  }, [checkInstalledApps]);

  const handleBack = useCallback(() => {
    setSelectedApp(null);
  }, []);

  const handleSelectApp = useCallback((app: AppInfo) => {
    setSelectedApp({ ...app });
  }, []);

  if (selectedApp) {
    return (
      <ErrorBoundary>
        <PackageDetails
          app={selectedApp}
          onBack={handleBack}
          onInstallStateChange={handleInstallStateChange}
        />
      </ErrorBoundary>
    );
  }

  return (
    <ErrorBoundary>
      <div className="space-y-8">
        <h1 className="text-4xl font-bold pl-3 pr-3">Mentioned Packages</h1>
        <ScrollArea className="h-[calc(100vh-10rem)]">
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
            {featuredApps.map((app, index) => (
              <Card
                key={index}
                className="h-full flex flex-col cursor-pointer hover:bg-muted transition-all duration-300"
                onClick={() => handleSelectApp(app)}
              >
                <CardHeader>
                  <div className="flex justify-between items-start">
                    <CardTitle>{app.name}</CardTitle>
                    {installedApps.has(app.name) && (
                      <Badge variant="secondary" className="text-sm">
                        Installed
                      </Badge>
                    )}
                  </div>
                </CardHeader>
                <CardContent className="flex-grow">
                  <CardDescription>
                    {app.description && app.description.length > 70
                      ? `${app.description.substring(0, 80)}...`
                      : app.description}
                  </CardDescription>
                  <p className="opacity-50 text-xs pt-2">
                    Version: {app.version}
                  </p>
                </CardContent>
                <CardFooter className="mt-1">
                  <Badge variant="secondary" className="text-sm">
                    {app.repository}
                  </Badge>
                </CardFooter>
              </Card>
            ))}
          </div>
        </ScrollArea>
      </div>
    </ErrorBoundary>
  );
};

export default Home;
