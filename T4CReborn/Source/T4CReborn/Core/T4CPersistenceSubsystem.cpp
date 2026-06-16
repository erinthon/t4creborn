#include "Core/T4CPersistenceSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

void UT4CPersistenceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Default local; pode ser sobrescrito em DefaultGame.ini [T4C.Persistence].
	BaseUrl = TEXT("http://127.0.0.1:8080");
	GConfig->GetString(TEXT("T4C.Persistence"), TEXT("BaseUrl"), BaseUrl, GGameIni);
	BaseUrl.TrimEndInline();
	if (BaseUrl.EndsWith(TEXT("/")))
	{
		BaseUrl.LeftChopInline(1);
	}
	UE_LOG(LogTemp, Display, TEXT("[T4C] Persistencia: BaseUrl=%s"), *BaseUrl);
}

FString UT4CPersistenceSubsystem::CharacterUrl(const FString& Id) const
{
	return FString::Printf(TEXT("%s/character?id=%s"), *BaseUrl, *Id);
}

void UT4CPersistenceSubsystem::SaveCharacter(const FString& Id, const FString& Json)
{
	if (!IsConfigured())
	{
		return;
	}
	const TSharedRef<IHttpRequest> Req = FHttpModule::Get().CreateRequest();
	Req->SetVerb(TEXT("PUT"));
	Req->SetURL(CharacterUrl(Id));
	Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Req->SetContentAsString(Json);
	Req->OnProcessRequestComplete().BindLambda(
		[Id](FHttpRequestPtr, FHttpResponsePtr Resp, bool bOk)
		{
			const int32 Code = (bOk && Resp.IsValid()) ? Resp->GetResponseCode() : 0;
			UE_LOG(LogTemp, Display, TEXT("[T4C] Salvou %s no servico (HTTP %d)"), *Id, Code);
		});
	Req->ProcessRequest();
}

void UT4CPersistenceSubsystem::LoadCharacter(const FString& Id, FT4COnCharacterLoaded OnLoaded)
{
	if (!IsConfigured())
	{
		OnLoaded.ExecuteIfBound(false, FString());
		return;
	}
	const TSharedRef<IHttpRequest> Req = FHttpModule::Get().CreateRequest();
	Req->SetVerb(TEXT("GET"));
	Req->SetURL(CharacterUrl(Id));
	Req->OnProcessRequestComplete().BindLambda(
		[OnLoaded, Id](FHttpRequestPtr, FHttpResponsePtr Resp, bool bOk)
		{
			if (bOk && Resp.IsValid() && Resp->GetResponseCode() == 200)
			{
				OnLoaded.ExecuteIfBound(true, Resp->GetContentAsString());
			}
			else
			{
				OnLoaded.ExecuteIfBound(false, FString());
			}
		});
	Req->ProcessRequest();
}
