#include "DynamicEncounterZones.h"

static void MessageHandler(SKSE::MessagingInterface::Message* message)
{
	switch (message->type) {
	case SKSE::MessagingInterface::kDataLoaded:
	{
		DynamicEncounterZones::GetSingleton()->CacheEncounterZones();
		break;
	}
	}
}

void Load()
{
	auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener("SKSE", MessageHandler);
	DynamicEncounterZones::InstallHooks();
}