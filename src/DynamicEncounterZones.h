#pragma once

#include <shared_mutex>

constexpr auto steamAppID = "72850";

class DynamicEncounterZones
{
public:
	static DynamicEncounterZones* GetSingleton()
	{
		static DynamicEncounterZones singleton;
		return &singleton;
	}

	static void InstallHooks();

	static RE::BGSEncounterZone* hk_GetEncounterZone(RE::TESObjectREFR* This);

	std::shared_mutex dataLock;
	bool dataLoaded = false;
	std::map<void*, RE::BGSEncounterZone*> locationToEncounterZoneMap;

	void CacheEncounterZones();
	RE::BGSEncounterZone* GetEncounterZone(RE::TESObjectREFR* This);

private:
	DynamicEncounterZones()
	{
	}

	DynamicEncounterZones(const DynamicEncounterZones&) = delete;
	DynamicEncounterZones(DynamicEncounterZones&&) = delete;

	~DynamicEncounterZones() = default;

	DynamicEncounterZones& operator=(const DynamicEncounterZones&) = delete;
	DynamicEncounterZones& operator=(DynamicEncounterZones&&) = delete;
};
