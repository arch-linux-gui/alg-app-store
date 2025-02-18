import React from "react";
import { Search, Package, PackagePlus, Home, Moon, Sun } from "lucide-react";
import { Button } from "@/components/ui/button";
import { useTheme } from "@/components/ui/theme-provider";

type PageType = "home" | "search" | "install" | "updates";

interface NavbarProps {
  setCurrentPage: (page: PageType) => void;
}

const Navbar: React.FC<NavbarProps> = ({ setCurrentPage }) => {
  const { setTheme, theme } = useTheme();

  return (
    <nav className="">
      <div className="container mx-auto px-4 py-4 flex items-center justify-between">
        <span
          className="text-2xl font-bold cursor-pointer"
          onClick={() => setCurrentPage("home")}
        >
          Arka Package Manager
        </span>
        <div className="flex items-center space-x-4">
          <Button variant="ghost" onClick={() => setCurrentPage("home")}>
            <Home className="mr-2 h-4 w-4" /> Home
          </Button>
          <Button variant="ghost" onClick={() => setCurrentPage("search")}>
            <Search className="mr-2 h-4 w-4" /> Search
          </Button>
          <Button variant="ghost" onClick={() => setCurrentPage("install")}>
            <Package className="mr-2 h-4 w-4" /> Installed
          </Button>
          <Button variant="ghost" onClick={() => setCurrentPage("updates")}>
            <PackagePlus className="mr-2 h-4 w-4" /> Updates
          </Button>
          <Button
            variant="ghost"
            size="icon"
            onClick={() => setTheme(theme === "dark" ? "light" : "dark")}
            className="rounded-full"
          >
            {theme === "dark" ? (
              <Sun className="h-[1.2rem] w-[1.2rem]" />
            ) : (
              <Moon className="h-[1.2rem] w-[1.2rem]" />
            )}
          </Button>
        </div>
      </div>
    </nav>
  );
};

export default Navbar;
