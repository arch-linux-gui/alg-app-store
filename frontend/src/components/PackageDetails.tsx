import React, {
  useEffect,
  useState,
  useCallback,
  useMemo,
  useRef,
} from "react";
import { Button } from "@/components/ui/button";
import { ArrowLeft, Download, Check, Copy, Trash2 } from "lucide-react";
import {
  Card,
  CardContent,
  CardDescription,
  CardFooter,
  CardHeader,
  CardTitle,
} from "@/components/ui/card";
import { Badge } from "@/components/ui/badge";
import { Separator } from "@/components/ui/separator";
import { ScrollArea } from "@/components/ui/scroll-area";
import { main } from "wailsjs/go/models";
import {
  CheckPackageInstalled,
  Install,
  Uninstall,
} from "../../wailsjs/go/main/App";
import ErrorBoundary from "./ErrorBoundary";
import { Skeleton } from "./ui/skeleton";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "./ui/tabs";
import CircularProgress from "./ui/circular-progress";

interface PackageDetailsProps {
  app: main.PackageInfo | null;
  onBack: () => void;
  onInstallStateChange: () => void;
}

const PackageDetails: React.FC<PackageDetailsProps> = ({
  app,
  onBack,
  onInstallStateChange,
}) => {
  const [isInstalled, setIsInstalled] = useState<boolean>(false);
  const [isCheckingInstall, setIsCheckingInstall] = useState<boolean>(false);
  const [isInstalling, setIsInstalling] = useState<boolean>(false);
  const [installProgress, setInstallProgress] = useState<number>(0);
  const [error, setError] = useState<string | null>(null);
  const [isLoading, setIsLoading] = useState<boolean>(true);
  const installInstructionsRef = useRef<HTMLDivElement>(null);
  const [copiedCommand, setCopiedCommand] = useState<string | null>(null);

  const handleCopyCommand = (command: string) => {
    navigator.clipboard.writeText(command).then(() => {
      setCopiedCommand(command);
      setTimeout(() => setCopiedCommand(null), 2000);
    });
  };

  const checkIfInstalled = useCallback(async (appName: string) => {
    try {
      setIsCheckingInstall(true);
      setError(null);
      const isExist = await CheckPackageInstalled(appName);
      setIsInstalled(isExist);
    } catch (err) {
      console.error("Error checking package installation:", err);
    } finally {
      setIsCheckingInstall(false);
    }
  }, []);

  useEffect(() => {
    if (app?.name) {
      setIsLoading(true);
      checkIfInstalled(app.name).finally(() => setIsLoading(false));
    }
  }, [app, checkIfInstalled]);

  const handleInstall = useCallback(async () => {
    if (!app?.name) return;

    try {
      setIsInstalling(true);
      setInstallProgress(0);
      setError(null);

      const installationInterval = setInterval(() => {
        setInstallProgress((prev) => Math.min(prev + 10, 90));
      }, 500);

      await Install(app.name);

      clearInterval(installationInterval);
      setInstallProgress(100);
      await checkIfInstalled(app.name);
      onInstallStateChange();
    } catch (err) {
      setError("Failed to install package");
      console.error("Error installing package:", err);
    } finally {
      const isExist = await CheckPackageInstalled(app.name);
      setIsInstalled(isExist);
      setIsInstalling(false);
    }
  }, [app, onInstallStateChange]);

  const handleUninstall = async () => {
    if (!app?.name) return;

    try {
      setIsInstalling(true);
      setInstallProgress(0);
      setError(null);

      const installationInterval = setInterval(() => {
        setInstallProgress((prev) => Math.min(prev + 10, 90));
      }, 500);

      await Uninstall(app.name);

      clearInterval(installationInterval);
      setInstallProgress(100);
      await checkIfInstalled(app.name);
      onInstallStateChange();
    } catch (err) {
      setError("Failed to install package");
      console.error("Error installing package:", err);
    } finally {
      const isExist = await CheckPackageInstalled(app.name);
      setIsInstalled(isExist);
      setIsInstalling(false);
    }
  };

  const renderDependencies = useMemo(() => {
    const dependencies = app?.dependlist;
    if (!dependencies || dependencies.length === 0) {
      return <p>No dependencies listed.</p>;
    }
    return (
      <ul className="list-disc pl-5">
        {dependencies.map((dependency) => (
          <li key={dependency}>{dependency}</li>
        ))}
      </ul>
    );
  }, [app?.dependlist]);

  const renderInstallButton = useMemo(() => {
    if (isInstalled) {
      return (
        <Button className="" onClick={handleUninstall}>
          <Trash2 className="mr-2 h-4 w-4" /> Uninstall
        </Button>
      );
    }

    if (isInstalling) {
      return (
        <Button disabled className="w-32">
          <CircularProgress percentage={installProgress} />
          Installing...
        </Button>
      );
    }
    return (
      <Button onClick={handleInstall}>
        <Download className="mr-2 h-4 w-4" /> Install
      </Button>
    );
  }, [
    isCheckingInstall,
    isInstalled,
    isInstalling,
    installProgress,
    handleInstall,
  ]);

  if (!app) {
    return <div>No package information available</div>;
  }

  if (isLoading) {
    return (
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6 pl-2 pr-2">
        {Array.from({ length: 9 }, (_, i) => (
          <Card
            key={i}
            className="h-[15rem] flex flex-col hover:bg-muted transition-all duration-300"
          >
            <CardHeader>
              <Skeleton className="h-9 w-[250px]" />
            </CardHeader>
            <CardContent>
              <Skeleton className="h-4 w-[250px]" />
              <Skeleton className="h-4 w-[200px] mt-2" />
            </CardContent>
            <CardFooter className="mt-10">
              <Skeleton className="h-7 rounded-full w-[50px]" />
            </CardFooter>
          </Card>
        ))}
      </div>
    );
  }

  const renderInstallationInstructions = () => {
    const isOfficialRepo =
      app.repository.includes("core") ||
      app.repository.includes("extra") ||
      app.repository.includes("local");
    const installCommand = `${isOfficialRepo ? "sudo pacman" : "yay"} -S ${
      app.name
    }`;

    const uninstallCommand = `sudo pacman -Rdd ${app.name}`;

    return (
      <div className="rounded-lg p-4" ref={installInstructionsRef}>
        <Tabs
          defaultValue={isOfficialRepo ? "pacman" : "yay"}
          className="w-full"
        >
          <TabsList className="mb-2">
            {isOfficialRepo ? (
              <TabsTrigger value="pacman">pacman</TabsTrigger>
            ) : (
              <>
                <TabsTrigger value="yay">yay</TabsTrigger>
                <TabsTrigger value="paru">paru</TabsTrigger>
              </>
            )}
          </TabsList>
          {isOfficialRepo ? (
            <div className=" flex">
              <TabsContent value="pacman" className="relative w-full">
                <pre className="bg-muted p-3 rounded-lg">
                  <code>
                    {app.repository.includes("local")
                      ? uninstallCommand
                      : installCommand}
                  </code>
                </pre>
                <CopyButton
                  command={installCommand}
                  copiedCommand={copiedCommand}
                  onCopy={handleCopyCommand}
                />
              </TabsContent>
            </div>
          ) : (
            <div className="flex">
              <TabsContent value="yay" className="relative w-full">
                <pre className="bg-muted p-3 rounded-lg">
                  <code>{installCommand}</code>
                </pre>
                <CopyButton
                  command={installCommand}
                  copiedCommand={copiedCommand}
                  onCopy={handleCopyCommand}
                />
              </TabsContent>
              <TabsContent value="paru" className="relative w-full">
                <pre className="bg-muted p-3 rounded-lg">
                  <code>{`paru -S ${app.name}`}</code>
                </pre>
                <CopyButton
                  command={`paru -S ${app.name}`}
                  copiedCommand={copiedCommand}
                  onCopy={handleCopyCommand}
                />
              </TabsContent>
              {/* <TooltipProvider>
                <Tooltip>
                  <TooltipTrigger>
                    <Button
                      className="mt-1.5 ml-2 p-3"
                      onClick={async () =>
                        await InstallAUR(aurManager, app.name)
                      }
                    >
                      <Terminal className="h-4 w-4" />
                    </Button>
                  </TooltipTrigger>
                  <TooltipContent>
                    <p>Install {app.name}</p>
                  </TooltipContent>
                </Tooltip>
              </TooltipProvider> */}
            </div>
          )}
        </Tabs>
      </div>
    );
  };

  return (
    <ErrorBoundary>
      <ScrollArea className="h-[calc(100vh-10rem)]">
        <div className="container mx-auto py-6 space-y-6">
          <div className="flex justify-between items-center">
            <Button variant="ghost" onClick={onBack} className="p-3">
              <ArrowLeft className="mr-2 h-4 w-4" /> Back
            </Button>
            {renderInstallButton}
          </div>
          {error && (
            <div
              className="bg-red-100 border border-red-400 text-red-700 px-4 py-3 rounded relative"
              role="alert"
            >
              <strong className="font-bold">Error: </strong>
              <span className="block sm:inline">{error}</span>
            </div>
          )}
          <Card>
            <CardHeader>
              <div className="flex justify-between items-start">
                <div>
                  <CardTitle className="text-3xl">
                    {app.name || "N/A"}
                  </CardTitle>
                  <CardDescription>
                    {app.description || "No description available."}
                  </CardDescription>
                </div>
                <Badge variant="secondary" className="text-lg pb-1.5 px-3">
                  {app.repository || "Unknown"}
                </Badge>
              </div>
            </CardHeader>
            <CardContent>
              <div className="grid grid-cols-2 gap-4">
                {[
                  { label: "Version", value: app.version },
                  { label: "Maintainer", value: app.maintainer },
                  { label: "Last Updated", value: app.lastupdated },
                  { label: "Upstream URL", value: app.upstreamurl },
                ].map(({ label, value }) => (
                  <div key={label}>
                    <p className="text-sm text-muted-foreground">{label}</p>
                    <p className="font-medium">{value || "N/A"}</p>
                  </div>
                ))}
              </div>
            </CardContent>
          </Card>
          <Card>
            <CardHeader>
              <CardTitle>Details</CardTitle>
            </CardHeader>
            <CardContent>
              <p>{app.description || "No description available."}</p>
              <Separator className="my-4" />
              <h3 className="font-semibold mb-2">Dependencies</h3>
              {renderDependencies}
              <Separator className="my-4" />
              <h3 className="font-semibold mb-2">Command</h3>
              {renderInstallationInstructions()}
            </CardContent>
            <CardFooter>
              <p className="text-sm text-muted-foreground">
                Please ensure your system meets the minimum requirements before
                installation.
              </p>
            </CardFooter>
          </Card>
        </div>
      </ScrollArea>
    </ErrorBoundary>
  );
};

interface CopyButtonProps {
  command: string;
  copiedCommand: string | null;
  onCopy: (command: string) => void;
}

const CopyButton: React.FC<CopyButtonProps> = ({
  command,
  copiedCommand,
  onCopy,
}) => (
  <Button
    variant="ghost"
    size="icon"
    className="absolute top-1 right-1 transition-all"
    onClick={() => onCopy(command)}
  >
    {copiedCommand === command ? (
      <Check className="h-4 w-4 text-green-500" />
    ) : (
      <Copy className="h-4 w-4" />
    )}
  </Button>
);

export default PackageDetails;
