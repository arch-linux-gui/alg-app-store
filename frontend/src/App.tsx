import React, { useState } from "react";
import { ThemeProvider } from "./components/ui/theme-provider";
import Navbar from "./components/Navbar";
import Home from "./components/Home";
import Search from "./components/Search";
import Install from "./components/Installed";
import { Toaster } from "./components/ui/toaster";
import Updates from "./components/Updates";

type PageType = "home" | "search" | "install" | "updates";

const App: React.FC = () => {
  const [currentPage, setCurrentPage] = useState<PageType>("home");
  const renderPage = () => {
    switch (currentPage) {
      case "home":
        return <Home />;
      case "search":
        return <Search />;
      case "install":
        return <Install />;
      case "updates":
        return <Updates />;
      default:
        return <Home />;
    }
  };

  return (
    <ThemeProvider defaultTheme="dark" storageKey="arch-pm-theme">
      <div className="min-h-screen bg-background text-foreground select-none">
        <Navbar setCurrentPage={setCurrentPage} />
        <main className="container mx-auto px-4 py-8">{renderPage()}</main>
        <Toaster />
      </div>
    </ThemeProvider>
  );
};

export default App;
