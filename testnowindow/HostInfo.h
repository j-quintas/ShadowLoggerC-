#pragma once
class HostInfo
{
public:
	HostInfo();
	~HostInfo();
	static void UserName();
	static void ComputerName();
	static void CPUArchitecture();
	static void IdleTime();
	static void SystemRAMUsedPercentage();
	static void ProcessStartTime();
	static void IsProcessAdmin();
	static void IsUserAdmin();
	static void PowerStatus();
	static void OSVersion();
	static void ProcessThreadCount();
	static void ProcessRAMUsage();
	static void LocalTime();
	static void ProcessPath();
	static void ExternalIP();
	static void ProductKey();
	static void UACStatus();
	static void ComputerType();
};

