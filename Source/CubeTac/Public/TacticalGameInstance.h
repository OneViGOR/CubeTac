// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Online.h"
#include "Engine/GameInstance.h"
#include "TacticalGameInstance.generated.h"


/**
*  PLEASE NOTE
*
*  This class is a work-in-progress attempt to move network-handling functions to C++. It is not yet fully functional.
*/

UCLASS()
class CUBETAC_API UTacticalGameInstance : public UGameInstance
{
	GENERATED_BODY()
	

public:
	/**
	*   FUNCTIONS
	*/

	UTacticalGameInstance(const FObjectInitializer& ObjectInitializer);

	/**
	*	Function to host a game
	*
	*	@Param		UserID				User that started the request
	*	@Param		SessionName			Name of the Session
	*	@Param		bIsLAN				Is this is LAN Game?
	*	@Param		bIsPresence			"Is the Session to create a presence Session"
	*	@Param		MaxNumPlayers	    Number of Maximum allowed players on this "Session" (Server)
	*/
	bool CreateGameSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers);
	

	UFUNCTION(BlueprintCallable, Category = "Network|Test")
		void StartOnlineGame();

	/**
	*	Function fired when a session create request has completed
	*
	*	@param		SessionName			The name of the session this callback is for
	*	@param		bWasSuccessful		True if the async action completed without error, false if there was an error
	*/
	UFUNCTION()
		virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	*	Function fired when a session start request has completed
	*
	*	@param		SessionName			The name of the session this callback is for
	*	@param		bWasSuccessful		True if the async action completed without error, false if there was an error
	*/
	UFUNCTION()
		void OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful);
	
	
	
	UFUNCTION()
		virtual void OnDestroySessionComplete(FName SessionName, bool bSuccess);


	/**
	*   VARIABLES
	*/

	// Session Handling

	TSharedPtr<class FOnlineSessionSettings> SessionSettings;

	/* Delegate called when session created */
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	/* Delegate called when session started */
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	/* Delegate called when session destroyed */
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;

	/** Handles to registered delegates for creating/starting a session */
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;

	/** Handles to registered delegates for destroying/ending a session */
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
};
