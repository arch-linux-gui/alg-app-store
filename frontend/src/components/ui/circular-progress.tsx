import { cn } from "@/lib/utils";
import React from "react";

interface CircularProgressProps {
  size?: number;
  strokeWidth?: number;
  percentage: number;
}

const CircularProgress: React.FC<CircularProgressProps> = ({
  size = 24,
  strokeWidth = 2,
  percentage,
}) => {
  const radius = (size - strokeWidth) / 2;
  const circumference = radius * 2 * Math.PI;
  const strokeDashoffset = circumference - (percentage / 100) * circumference;

  return (
    <svg
      width={size}
      height={size}
      viewBox={`0 0 ${size} ${size}`}
      className="rotate-[-90deg] mr-2"
    >
      <circle
        className="text-gray-300"
        strokeWidth={strokeWidth}
        stroke="currentColor"
        fill="transparent"
        r={radius}
        cx={size / 2}
        cy={size / 2}
      />
      <circle
        className="text-blue-600"
        strokeWidth={strokeWidth}
        strokeDasharray={circumference}
        strokeDashoffset={strokeDashoffset}
        strokeLinecap="round"
        stroke="currentColor"
        fill="transparent"
        r={radius}
        cx={size / 2}
        cy={size / 2}
      />
    </svg>
  );
};

export default CircularProgress;
