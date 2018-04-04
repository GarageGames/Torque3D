#include "GOAPAI_GoalManager.h"

IMPLEMENT_STATIC_CLASS(GOAPAI_GoalManager, , "Functions for maintaing a list of banned users.");

ConsoleDoc(
	"@class GOAPAI_GoalManager\n"
	"@ingroup Miscellaneous\n"
	"@brief Used for kicking and banning players from a server.\n"
	"There is only a single instance of BanList. It is very important to note that you do not ever create this object in script "
	"like you would other game play objects. You simply reference it via namespace.\n\n"
	"For this to be used effectively, make sure you are hooking up other functions to BanList. "
	"For example, functions like GameConnection::onConnectRequestRejected( %this, %msg ) and function GameConnection::onConnectRequest are excellent "
	"places to make use of the BanList. Other systems can be used in conjunction for strict control over a server\n\n"
	"@see addBadWord\n"
	"@see containsBadWords\n"
);

GOAPAI_GoalManager* GOAPAI_GoalManager::smInstance;

GOAPAI_GoalManager::GOAPAI_GoalManager()
{
	smInstance = this;
}

DefineEngineStaticMethod(GOAPAI_GoalManager, add, void, (), , "")
{
	//GOAPAI_GoalManager::instance()->add();
}