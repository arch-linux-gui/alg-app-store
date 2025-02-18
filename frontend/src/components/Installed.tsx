import React, { useState, useEffect, useCallback, useMemo } from "react";
import { FixedSizeGrid as Grid } from "react-window";
import AutoSizer from "react-virtualized-auto-sizer";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import {
  Card,
  CardContent,
  CardDescription,
  CardFooter,
  CardHeader,
  CardTitle,
} from "@/components/ui/card";
import { GetInstalledPackages } from "../../wailsjs/go/main/App";
import { main } from "wailsjs/go/models";
import { Badge } from "./ui/badge";
import { Skeleton } from "./ui/skeleton";
import PackageDetails from "./PackageDetails";
import { Alert, AlertDescription, AlertTitle } from "@/components/ui/alert";
import { AlertCircle } from "lucide-react";

const Installed = () => {
  const [installedPackages, setInstalledPackages] = useState<
    main.PackageInfo[]
  >([]);
  const [isLoading, setIsLoading] = useState<boolean>(true);
  const [error, setError] = useState<string | null>(null);
  const [searchTerm, setSearchTerm] = useState<string>("");
  const [selectedPackage, setSelectedPackage] =
    useState<main.PackageInfo | null>(null);

  const fetchInstalledPackages = useCallback(async () => {
    setIsLoading(true);
    try {
      const packages = await GetInstalledPackages();
      setInstalledPackages(packages);
      setError(null);
    } catch (err) {
      console.error("Error fetching installed packages:", err);
      setError("Failed to fetch installed packages. Please try again.");
    } finally {
      setIsLoading(false);
    }
  }, []);

  useEffect(() => {
    fetchInstalledPackages();
    const intervalId = setInterval(fetchInstalledPackages, 5000);
    return () => clearInterval(intervalId);
  }, [fetchInstalledPackages]);

  const filteredPackages = useMemo(() => {
    return installedPackages.filter((pkg) =>
      pkg.name.toLowerCase().includes(searchTerm.toLowerCase())
    );
  }, [installedPackages, searchTerm]);

  const PackageItem = useCallback(
    ({
      columnIndex,
      rowIndex,
      style,
    }: {
      columnIndex: number;
      rowIndex: number;
      style: React.CSSProperties;
    }) => {
      const index = rowIndex * 3 + columnIndex;
      const pkg = filteredPackages[index];
      if (!pkg) return null;

      return (
        <div style={style} className="p-2">
          <Card
            className="h-full flex flex-col cursor-pointer hover:bg-muted transition-all duration-300"
            onClick={() => setSelectedPackage(pkg)}
          >
            <CardHeader>
              <CardTitle>{pkg.name}</CardTitle>
            </CardHeader>
            <CardContent className="flex-grow">
              <CardDescription>
                {pkg.description && pkg.description.length > 70
                  ? `${pkg.description.substring(0, 80)}...`
                  : pkg.description}
              </CardDescription>
              <p className="opacity-50 text-xs pt-2">
                Last Updated: {pkg.lastupdated}
              </p>
            </CardContent>
            <CardFooter>
              <Badge variant="secondary" className="text-sm">
                {pkg.version}
              </Badge>
            </CardFooter>
          </Card>
        </div>
      );
    },
    [filteredPackages]
  );

  const renderContent = useMemo(() => {
    if (isLoading && installedPackages.length === 0) {
      return (
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6 pl-2 pr-2">
          {Array.from({ length: 9 }, (_, i) => (
            <Card key={i} className="h-[15rem] flex flex-col">
              <CardHeader>
                <Skeleton className="h-9 w-[250px]" />
              </CardHeader>
              <CardContent>
                <Skeleton className="h-4 w-[250px]" />
                <Skeleton className="h-4 w-[200px] mt-2" />
              </CardContent>
              <CardFooter className="mt-10">
                <Skeleton className="h-7 w-[80px]" />
              </CardFooter>
            </Card>
          ))}
        </div>
      );
    }

    if (error) {
      return (
        <Alert variant="destructive">
          <AlertCircle className="h-4 w-4" />
          <AlertTitle>Error</AlertTitle>
          <AlertDescription>{error}</AlertDescription>
        </Alert>
      );
    }

    if (filteredPackages.length === 0) {
      return (
        <div className="p-3">
          <Alert>
            <AlertTitle className="font-bold text-lg">
              No packages found
            </AlertTitle>
            <AlertDescription>
              {searchTerm
                ? "No packages match your search."
                : "No installed packages found."}
            </AlertDescription>
          </Alert>
        </div>
      );
    }

    return (
      <div className="h-[calc(100vh-200px)]">
        <AutoSizer>
          {({ height, width }: { height: number; width: number }) => {
            const columnCount = width >= 1024 ? 3 : width >= 768 ? 2 : 1;
            const columnWidth = width / columnCount;
            const rowCount = Math.ceil(filteredPackages.length / columnCount);
            return (
              <Grid
                columnCount={columnCount}
                columnWidth={columnWidth}
                height={height}
                rowCount={rowCount}
                rowHeight={250}
                width={width}
              >
                {PackageItem}
              </Grid>
            );
          }}
        </AutoSizer>
      </div>
    );
  }, [
    isLoading,
    error,
    filteredPackages,
    PackageItem,
    installedPackages.length,
    searchTerm,
  ]);

  return (
    <div className="space-y-6 h-full">
      {selectedPackage ? (
        <PackageDetails
          app={selectedPackage}
          onBack={() => setSelectedPackage(null)}
          onInstallStateChange={fetchInstalledPackages}
        />
      ) : (
        <>
          <h1 className="text-4xl font-bold pl-3 pr-3">Installed Packages</h1>
          <div className="flex space-x-4 pl-3 pr-3">
            <Input
              type="text"
              placeholder="Search installed packages..."
              value={searchTerm}
              onChange={(e: React.ChangeEvent<HTMLInputElement>) =>
                setSearchTerm(e.target.value)
              }
              className="flex-grow"
              aria-label="Search installed packages"
            />
          </div>
          {renderContent}
        </>
      )}
    </div>
  );
};

export default React.memo(Installed);
