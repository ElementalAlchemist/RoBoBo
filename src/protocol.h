#pragma once
#include "main.h"

#include "socket.h"

class Base;

/// Define the spawn function for protocol module files
#define PROTOCOL_SPAWN(modName) extern "C" Protocol* spawn(std::string server, std::map<std::string, std::string> conf, std::string workDir, bool dumpLogs, unsigned short debug, Base* botptr) {\
	return new modName (server, conf, workDir, dumpLogs, debug, botptr);\
}

class Protocol {
	public:
		/** Protocol constructor
		 * Fills the Protocol class with the basic things it needs to function
		 * @param server The name of the server being loaded; also serves as the server address
		 * @param conf The server configuration
		 * @param workDir The directory of the bot files relative to where the bot is started from
		 * @param dumpLogs Whether the --log argument was given to indicate that logs should be output
		 * @param debug The debug level at which the bot is running
		 * @param botptr A pointer to the active Base class allowing the protocol module to call hooks of bot modules
		 */
		Protocol(std::string server, std::map<std::string, std::string> conf, std::string workDir, bool dumpLogs, unsigned short debug, Base* botptr);
		
		/** Protocol destructor
		 * As this is the class from which Protocol modules are derived, this is virtual.
		 */
		virtual ~Protocol();
		
		/** API version check
		 * This function must be defined by implementing modules.
		 * It serves to check that the protocol module API for which the module in question was written is the
		 * one in use by the bot core.  If not, the module is unloaded.
		 * @return API version
		 */
		virtual unsigned int apiVersion() = 0;
		
		/** Connect to server
		 * This function is called by the core when everything is ready for the protocol module to connect to the server.
		 */
		virtual void connectServer();
		
		/** Disconnect from server
		 * This function is called by the core if, for some reason, the bot needs to disconnect from the server.
		 * @param reason The reason to disconnect
		 */
		virtual void disconnectServer(std::string reason);
		
		/** Connection check
		 * This function is used to determine if the connection to the server is still active.
		 * @return true if the connection is still active
		 */
		virtual bool isConnected();
		
		/** Dead server check
		 * This function is used by the server check thread to determine if the server should be unloaded.
		 * @return true to unload the server
		 */
		virtual bool deadServer();
		
		/** Client check
		 * Checks whether this type of protocol module connects as a client.
		 * @return true if it uses a client connection, false if it uses a server-to-server connection
		 */
		virtual bool isClient();
		
		/** End debug
		 * If, at any time, the bot leaves debug mode, this function resets the debug level to 0.
		 * @see debugLevel
		 */
		void endDebug();
		
		
		/** Send message
		 * This function is called when something wants to send a message on this server.
		 * @param client The identifier of the client to send the message
		 * @param target Where to send the message
		 * @param message The message to send
		 */
		virtual void sendPrivMsg(std::string client, std::string target, std::string message);
		
		/** Send notice
		 * This function is called when something wants to send a notice on this server.
		 * @param client The identifier of the client to send the notice
		 * @param target Where to send the notice
		 * @param message The message to send in the notice
		 */
		virtual void sendNotice(std::string client, std::string target, std::string message);
		
		/** Send CTCP request
		 * This function is called when something wants to send a CTCP request on this server.
		 * @param client The identifier of the client to send the CTCP request
		 * @param target Where to send the CTCP request
		 * @param ctcp The CTCP request body, including the type and any parameters
		 */
		virtual void sendCTCP(std::string client, std::string target, std::string ctcp, std::string params);
		
		/** Send CTCP reply
		 * This function is called when something wants to send a CTCP reply on this server.
		 * @param client The identifier of the client to send the CTCP reply
		 * @param target Where to send the CTCP reply
		 * @param ctcp The CTCP reply body, including the type and any data
		 */
		virtual void sendCTCPReply(std::string client, std::string target, std::string ctcp, std::string params);
		
		/** Set modes
		 * This function is called when something wants to set modes on this server.
		 * @param client The identifier of the client to set the modes
		 * @param target The user or channel on which to set modes
		 * @param setModes A list of long-name modes to set
		 * @param delModes A list of long-name modes to unset
		 */
		virtual void setMode(std::string client, std::string target, std::list<std::string> setModes, std::list<std::string> delModes);
		
		/** Set SNOmask
		 * This function is called when something wants to change the SNOmask on this server.
		 * @param client The identifier of the client whose SNOmasks should be changed
		 * @param snomask The SNOmask char to modify
		 * @param add True if the SNOmask should be set, or false to unset
		 */
		virtual void setSNOMask(std::string client, char snomask, bool add);
		
		/** Set channel topic
		 * This function is called when something wants to set a topic on this server.
		 * @param client The identifier of the client to set the topic
		 * @param channel The channel on which to change the topic
		 * @param topic The new channel topic
		 */
		virtual void setChanTopic(std::string client, std::string channel, std::string topic);
		
		/** Join channel
		 * This function is called when something wants a client to join a channel on this server.
		 * @param client The identifier of the client to join the channel
		 * @param channel The channel to join
		 * @param key The channel key, if required
		 */
		virtual void joinChannel(std::string client, std::string channel, std::string key = "");
		
		/** Part channel
		 * This function is called when something wants a client to part a channel on this server.
		 * @param client The identifier of the client to part the channel
		 * @param channel The channel to part
		 * @param reason The part reason
		 */
		virtual void partChannel(std::string client, std::string channel, std::string reason);
		
		/** Kick user
		 * This function is called when something wants a client to kick a user on this server.
		 * @param client The identifier of the client to perform the kick
		 * @param channel The channel from which to kick the user
		 * @param nick The nick of the user to kick
		 * @param reason The kick reason
		 */
		virtual void kickUser(std::string client, std::string channel, std::string nick, std::string reason);
		
		/** Change nick
		 * This function is called when something wants a client to change nicks.
		 * @param client The identifier of the client to change nicks
		 * @param newNick The desired new nick
		 */
		virtual void changeNick(std::string client, std::string newNick);
		
		/** Add new client
		 * This function is called when something wants to introduce a new client to the server.
		 * @param nick The nick of the new client
		 * @param ident The ident of the new client
		 * @param host The host of the new client
		 * @param gecos The gecos of the new client
		 * @return An identifier that can be used to refer to this client; an empty string if something went wrong
		 */
		virtual std::string addClient(std::string nick, std::string ident, std::string host, std::string gecos);
		
		/** Remove client
		 * This function is called when something wants to remove an existing client from the server.
		 * @param client The identifier of the client to remove.
		 */
		virtual void removeClient(std::string client);
		
		/** Oper up
		 * This function is called when something wants a client to oper up on this server.
		 * @param client The identifier of the client to oper up
		 * @param username Username to oper up as, or various uses by server type
		 * @param password The oper password, or varies by server type
		 */
		virtual void oper(std::string client, std::string username, std::string password);
		
		/** Send SNOtice
		 * This function is called when something wants to send a server notice on this server.
		 * @param snomask The SNOmask type this message is for
		 * @param message The message to send
		 */
		virtual void sendSNotice(char snomask, std::string message);
		
		/** Set metadata
		 * This function is called when something wants to change metadata of a user or channel on this server.
		 * @param target The user or channel of which to change the metadata
		 * @param key The metadata type to change
		 * @param value The new value of the metadata
		 */
		virtual void setMetadata(std::string target, std::string key, std::string value);
		
		/** Set x:line
		 * This function is called when something wants to set an x:line on this server.
		 * @param client The identifier of the client to set the x:line
		 * @param linetype The type of x:line to set
		 * @param mask The mask on which to set the x:line
		 * @param duration The duration of the x:line in seconds (or 0 for permanent on servers that support it)
		 * @param reason The reason for the x:line
		 */
		virtual void setXLine(std::string client, std::string linetype, std::string mask, time_t duration, std::string reason);
		
		/** Remove x:line
		 * This function is called when something wants to remove an x:line on this server.
		 * @param client The identifier of the client to remove the x:line
		 * @param linetype The type of x:line to remove
		 * @param mask The mask on which to remove the x:line
		 */
		virtual void delXLine(std::string client, std::string linetype, std::string mask);
		
		/** Send other data
		 * This function is called when something wants to send data to the server that cannot be handled by another API function.
		 * @param client The identifier of the client to send the data
		 * @param line The raw line to send
		 */
		virtual void sendOtherData(std::string client, std::string line);
		
		
		/** Processed channel message hook
		 * This function is called when the outgoing channel message hooks have all been called.
		 * @param client The identifier of the client to send the message
		 * @param target The channel to which to send the message
		 * @param status The status char to which to send the message, e.g. '@' if it is to go to @#channel
		 * @param message The message to send
		 */
		virtual void processedChanMsg(std::string client, std::string target, char status, std::string message);
		
		/** Processed user message hook
		 * This function is called when the outgoing user message hooks have all been called.
		 * @param client The identifier of the client to send the message
		 * @param target The nick of the user to whom to send the message
		 * @param message The message to send
		 */
		virtual void processedUserMsg(std::string client, std::string target, std::string message);
		
		/** Processed channel notice hook
		 * This function is called when the outgoing channel notice hooks have all been called.
		 * @param client The identifier of the client to send the message
		 * @param target The channel to which to send the message
		 * @param status The status char to which to send the message, e.g. '@' if it is to go to @#channel
		 * @param message The message to send
		 */
		virtual void processedChanNotice(std::string client, std::string target, char status, std::string message);
		
		/** Processed user notice hook
		 * This function is called when the outgoing user notice hooks have all been called.
		 * @param client The identifier of the client to send the message
		 * @param target The nick of the user to whom to send the message
		 * @param message The message to send
		 */
		virtual void processedUserNotice(std::string client, std::string target, std::string message);
		
		/** Processed channel CTCP request hook
		 * This function is called when the outgoing channel CTCP request hooks have all been called.
		 * @param client The identifier of the client to send the request
		 * @param target The channel to which to send the request
		 * @param status The status char to which to send the request, e.g. '@' if it is to go to @#channel
		 * @param ctcp The CTCP type and any associated parameters
		 */
		virtual void processedChanCTCP(std::string client, std::string target, char status, std::string ctcp);
		
		/** Processed user CTCP request hook
		 * This function is called when the outgoing user CTCP request hooks have all been called.
		 * @param client The identifier of the client to send the request
		 * @param target The nick of the user to whom to send the request
		 * @param ctcp The CTCP type and any associated parameters
		 */
		virtual void processedUserCTCP(std::string client, std::string target, std::string ctcp);
		
		/** Processed channel CTCP reply hook
		 * This function is called when the outgoing channel CTCP reply hooks have all been called.
		 * @param client The identifier of the client to send the reply
		 * @param target The channel to which to send the reply
		 * @param status The status char to which to send the reply, e.g. '@' if it is to go to @#channel
		 * @param ctcp The CTCP type and any associated data
		 */
		virtual void processedChanCTCPReply(std::string client, std::string target, char status, std::string ctcp);
		
		/** Processed user CTCP reply hook
		 * This function is called when the outgoing user CTCP reply hooks have all been called.
		 * @param client The identifier of the client to send the reply
		 * @param target The nick of the user to whom to send the reply
		 * @param ctcp The CTCP type and any associated data
		 */
		virtual void processedUserCTCPReply(std::string client, std::string target, std::string ctcp);
		
		
		/** List mode list
		 * Lists list modes available on this server.
		 * @return a list of long-name modes
		 */
		virtual std::list<std::string> listModes();
		
		/** Parameter mode list
		 * Lists modes that take parameters that are available on this server.
		 * @return a list of long-name modes
		 */
		virtual std::list<std::string> paramModes();
		
		/** Mode list
		 * Lists parameterless modes available on this server.
		 * @return a list of long-name modes
		 */
		virtual std::list<std::string> modes();
		
		/** Statuses list
		 * Lists status ranks available on this server.
		 * @return a list of long-name modes and status chars
		 */
		virtual std::list<std::pair<std::string, char>> statuses();
		
		/** Channel list
		 * Lists channels on this server.
		 * @return a list of channels
		 */
		virtual std::list<std::string> channels();
		
		/** Client channel list
		 * Lists channels in which the given client is.
		 * @param client The client of which we're checking the channel list
		 * @return a list of channels
		 */
		virtual std::list<std::string> inChannels(std::string client);
		
		/** Channel user list
		 * Lists users in the given channel
		 * @param channel The channel whose users to list
		 * @return a list of nicks
		 */
		virtual std::list<std::string> channelUsers(std::string channel);
		
		/** Channel user check
		 * Checks whether the given user is in the given channel.
		 * @param channel The channel in which to check for the user
		 * @param user The user for whom to check
		 * @return true if the user is in the channel, false otherwise
		 */
		virtual bool userInChannel(std::string channel, std::string user);
		
		/** Channel topic
		 * Shows the topic of the given channel.
		 * @param channel The channel whose topic to show
		 * @return channel topic
		 */
		virtual std::string channelTopic(std::string channel);
		
		/** Channel mode list
		 * Lists all non-list modes set in a channel.
		 * @param channel The channel whose modes to list
		 * @return a list of modes
		 */
		virtual std::list<std::string> channelModes(std::string channel);
		
		/** Channel mode check
		 * Checks whether the given non-list mode is set in the given channel.
		 * @param channel The channel in which to check for the mode
		 * @param mode The mode for which to check
		 * @return true if the mode is set, false otherwise
		 */
		virtual bool channelHasMode(std::string channel, std::string mode);
		
		/** Mode param
		 * Shows the parameter of the given mode on the given channel.
		 * @param channel The channel in which to check for the mode
		 * @param mode The mode of which to read the parameter
		 * @return the mode parameter, or an empty string if the mode is not set
		 */
		virtual std::string modeParam(std::string channel, std::string mode);
		
		/** Channel list mode list
		 * Shows the list for a given list mode on a given channel.
		 * @param channel The channel whose list mode list to show
		 * @param mode The list mode whose list to show
		 * @return a list of list mode entries
		 */
		virtual std::list<std::string> channelListMode(std::string channel, std::string mode);
		
		/** Channel list mode entry check
		 * Checks whether a channel list mode has a given entry.
		 * @param channel The channel whose lists to check
		 * @param listMode The list mode whose lists to check
		 * @param entry The entry for which to check
		 * @return true if the list has the entry, false otherwise
		 */
		virtual bool channelListHasEntry(std::string channel, std::string listMode, std::string entry);
		
		/** User status
		 * Shows the status rank of a given user on a given channel
		 * @param channel The channel to check for a status
		 * @param user The user whose status to check
		 * @return the long-name mode and status char of the user
		 */
		virtual std::pair<std::string, char> userStatus(std::string channel, std::string user);
		
		/** Compare status
		 * Compare the two status levels given.
		 * @param status0 A long-name mode or status char to compare
		 * @param status1 The other long-name mode or status char to compare
		 * @return The long-name mode and status char of the higher ranked status
		 */
		virtual std::pair<std::string, char> compareStatus(std::string status0, std::string status1);
		virtual std::pair<std::string, char> compareStatus(std::string status0, char status1);
		virtual std::pair<std::string, char> compareStatus(char status0, std::string status1);
		virtual std::pair<std::string, char> compareStatus(char status0, char status1);
		
		/** User status check
		 * Checks whether the given user has the given status on the given channel.
		 * @param channel The channel to check
		 * @param user The user to check
		 * @param status The status to check
		 * @return true if the user has the status, false otherwise
		 */
		virtual bool userHasStatus(std::string channel, std::string user, std::string status);
		virtual bool userHasStatus(std::string channel, std::string user, char status);
		
		/** User minimum status check
		 * Checks whether the given user has the given status or any higher status rank on the given channel.
		 * @param channel The channel to check
		 * @param user The user to check
		 * @param status The status to check
		 * @return true if the user has the status or any higher-ranking status, false otherwise
		 */
		virtual bool userHasStatusOrGreater(std::string channel, std::string user, std::string status);
		virtual bool userHasStatusOrGreater(std::string channel, std::string user, char status);
		
		/** Client list
		 * Shows which clients this protocol module is running
		 * @return a list of client identifiers
		 */
		virtual std::list<std::string> clients();
		
		/** User mode list
		 * Shows which user modes are set on the given client.
		 * @param client The client whose modes to check
		 * @return a list of long name modes
		 */
		virtual std::list<std::string> userModes(std::string client);
		
		/** User mode check
		 * Checks whether a given user mode is set on a given client.
		 * @param client The client to check
		 * @param mode The mode for which to check
		 * @return true if the mode is set, false otherwise
		 */
		virtual bool hasUserMode(std::string client, std::string mode);
		
		/** SNOmask list
		 * Lists SNOmasks set on a given client.
		 * @param client The client to check
		 * @return a list of SNOmask chars
		 */
		virtual std::list<char> snomasks(std::string client);
		
		/** SNOmask check
		 * Checks whether a given SNOmask is set on a given client.
		 * @param client The client to check
		 * @param snomask The SNOmask char for which to check
		 * @return true if the client has a given SNOmask, false otherwise
		 */
		virtual bool hasSNOMask(std::string client, char snomask);
		
		/** User channels list
		 * Lists channels in which the given user is.
		 * @param nick The nick of the user
		 * @return a list of channel names
		 */
		virtual std::list<std::string> userChannels(std::string nick);
	protected:
		/** Utility variables
		 * serverName: The name of the protocol module as used internally, and the server address
		 * workingDir: The directory of the bot files relative to where the bot was started from
		 */
		std::string serverName, workingDir;
		
		/// The server config
		std::map<std::string, std::string> config;
		
		/// Whether the --log argument was given signaling that the bot should log I/0 to output
		bool log;
		
		/// The debug level at which the bot is running
		unsigned short debugLevel;
		
		/** Socket generator
		 * This function loads and spawns a new socket for use with this protocol module.
		 * @param socketType The type of socket module to load
		 * @return A pointer to a new Socket instance
		 */
		Socket* assignSocket(std::string socketType);
		
		/** Socket destroyer
		 * This function must be called when you are done with your Socket so that it can be cleaned up properly.
		 * @param socketType The type of socket that it is (same as in assignSocket)
		 * @param sockptr A pointer to the socket
		 */
		void killSocket(std::string socketType, Socket* sockptr);
		
		
		/** Call channel message hook
		 * This function lets modules know that we've received a channel message from the server.
		 * @param client The identifier of the client that read the message
		 * @param channel The channel to which the message was sent
		 * @param status The status char to which the message was sent, e.g. '@' if the message was sent to @#channel; this
		 * should be a blank space (' ') if it was sent to the entire channel
		 * @param nick The nick of the user who sent the message
		 * @param message The message
		 */
		void callChanMsgHook(std::string client, std::string channel, char status, std::string nick, std::string message);
		
		/** Call user message hook
		 * This function lets modules know that we've received a user notice from the server.
		 * @param client The identifier of the client that received the message
		 * @param nick The nick of the user who sent the message
		 * @param message The message
		 */
		void callUserMsgHook(std::string client, std::string nick, std::string message);
		
		/** Call channel notice hook
		 * This function lets modules know that we've received a channel notice from the server.
		 * @param client The identifier of the client that read the notice
		 * @param channel The channel to which the notice was sent
		 * @param status The status char to which the notice was sent, e.g. '@' if the notice was sent to @#channel; this
		 * should be a blank space (' ') if it was sent to the entire channel
		 * @param nick The nick of the user who sent the notice
		 * @param message The message
		 */
		void callChanNoticeHook(std::string client, std::string channel, char status, std::string nick, std::string message);
		
		/** Call user notice hook
		 * This function lets modules know that we've received a user notice from the server.
		 * @param client The identifier of the client that received the notice
		 * @param nick The nick of the user who sent the notice
		 * @param message The message
		 */
		void callUserNoticeHook(std::string client, std::string nick, std::string message);
		
		/** Call channel CTCP hook
		 * This function lets modules know that we've received a channel CTCP request from the server.
		 * @param client The identifier of the client that read the request
		 * @param channel The channel to which the request was sent
		 * @param status The status char to which the request was sent, e.g. '@' if the request was sent to @#channel; this
		 * should be a blank space (' ') if it was sent to the entire channel
		 * @param nick The nick of the user who sent the request
		 * @param ctcp The CTCP type with any parameters
		 */
		void callChanCTCPHook(std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string params);
		
		/** Call user CTCP hook
		 * This function lets modules know that we've received a user CTCP request from the server.
		 * @param client The identifier of the client that received the request
		 * @param nick The nick of the user who sent the request
		 * @param ctcp The CTCP type with any parameters
		 */
		void callUserCTCPHook(std::string client, std::string nick, std::string ctcp, std::string params);
		
		/** Call channel CTCP reply hook
		 * This function lets modules know that we've received a channel CTCP reply from the server.
		 * @param client The identifier of the client that read the reply
		 * @param channel The channel to which the reply was sent
		 * @param status The status char to which the reply was sent, e.g. '@' if the request was sent to @#channel; this
		 * should be a blank space (' ') if it was sent to the entire channel
		 * @param nick The nick of the user who sent the reply
		 * @param ctcp The CTCP type with any data
		 */
		void callChanCTCPReplyHook(std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string params);
		
		/** Call user CTCP reply hook
		 * This function lets modules know that we've received a user CTCP reply from the server.
		 * @param client The identifier of the client that received the reply
		 * @param nick The nick of the user who sent the reply
		 * @param ctcp The CTCP type with any data
		 */
		void callUserCTCPReplyHook(std::string client, std::string nick, std::string ctcp, std::string params);
		
		/** Call channel mode hook
		 * This function lets modules know that a channel's modes have been changed on the server.
		 * @param channel The channel whose modes were changed
		 * @param add Whether the mode was set (true) or unset (false)
		 * @param mode The long name of the mode set or unset
		 * @param param The mode parameter, or an empty string if no parameter was given
		 */
		void callChanModeHook(std::string channel, std::string setter, bool add, std::string mode, std::string param);
		
		/** Call user mode hook
		 * This function lets modules know that a user's modes have been changed on the server.
		 * @param nick The nick of the user whose modes have changed
		 * @param add Whether the mode has been set (true) or unset (false)
		 * @param mode The long name of the mode set or unset
		 */
		void callUserModeHook(std::string nick, bool add, std::string mode);
		
		/** Call user SNOmask hook
		 * This function lets modules know that a user's SNOmasks have changed on the server.
		 * @param nick The nick of the user whose SNOmasks have changed
		 * @param add Whether the snomask char has been set (true) or unset (false)
		 * @param snomask The snomask char set or unset
		 */
		void callUserSNOMaskHook(std::string nick, bool add, char snomask);
		
		/** Call channel topic hook
		 * This function lets modules know that a channel's topic has been changed on the server.
		 * @param channel The channel whose topic changed
		 * @param topic The new channel topic
		 */
		void callChanTopicHook(std::string channel, std::string setter, std::string topic);
		
		/** Call channel join hook
		 * This function lets modules know that a channel has been joined by a user on the server.
		 * @param channel The channel that was joined
		 * @param nick The nick of the user who joined
		 */
		void callChanJoinHook(std::string channel, std::string nick);
		
		/** Call channel part hook
		 * This function lets modules know that a channel has been parted by a user on the server.
		 * @param channel The channel that was parted
		 * @param nick The nick of the user who parted
		 * @param reason The part reason
		 */
		void callChanPartHook(std::string channel, std::string nick, std::string reason);
		
		/** Call channel kick hook
		 * This function lets modules know that a user has been kicked from a channel on the server.
		 * @param channel The channel from which the user was kicked
		 * @param kicker The nick of the user who kicked
		 * @param kickee The nick of the user who was kicked
		 * @param reason The kick reason
		 */
		void callChanKickHook(std::string channel, std::string kicker, std::string kickee, std::string reason);
		
		/** Call user nick hook
		 * This function lets modules know that a user changed nicks on the server.
		 * @param oldNick The old nick of the user
		 * @param newNick The new nick of the user
		 */
		void callUserNickHook(std::string oldNick, std::string newNick);
		
		/** Call user connect hook
		 * This function lets modules know that a user has connected to the server.
		 * @param nick The nick of the user who connected
		 */
		void callUserConnectHook(std::string nick);
		
		/** Call user quit hook
		 * This function lets modules know that a user has quit from the server.
		 * @param nick The nick of the user who quit
		 * @param reason The quit reason
		 */
		void callUserQuitHook(std::string nick, std::string reason);
		
		/** Call numeric hook
		 * This function lets modules know that a numeric has been received from the server.
		 * @param client The identifier of the client that received the numeric
		 * @param numeric The numeric received
		 * @param data Any parameters to the numeric, parsed into IRC tokens
		 */
		void callNumericHook(std::string client, std::string numeric, std::vector<std::string> data);
		
		/** Call oper hook
		 * This function lets modules know that a user has opered on a server.
		 * @param nick The nick of the user who opered
		 * @param operType The type of oper the user now is
		 */
		void callOperHook(std::string nick, std::string operType);
		
		/** Call SNOtice hook
		 * This function lets modules know that a server notice has been received from the server.
		 * @param snotype The SNOmask char under which the notice was received
		 * @param message The notice message
		 */
		void callSNoticeHook(char snotype, std::string message);
		
		/** Call metadata hook
		 * This function lets modules know that user or channel metadata has been changed on the server.
		 * @param target The user or channel whose metadata changed
		 * @param dataKey The type of metadata that changed
		 * @param dataValue The new value for the dataKey metadata
		 */
		void callMetadataHook(std::string target, std::string dataKey, std::string dataValue);
		
		/** Call x:line add hook
		 * This function lets modules know that an x:line has been set on the server.
		 * @param lineType The type of x:line set
		 * @param mask The mask on which the x:line was set
		 * @param setter The setter of the x:line
		 * @param expiry The timestamp of the expiry of the x:line
		 * @param reason The x:line reason
		 */
		void callXLineAddHook(std::string lineType, std::string mask, std::string setter, time_t expiry, std::string reason);
		
		/** Call x:line remove hook
		 * This function lets modules know that an x:line has been removed from or has expired on the server.
		 * @param lineType The type of x:line removed
		 * @param mask The mask on which the x:line was
		 */
		void callXLineRemoveHook(std::string lineType, std::string mask);
		
		/** Call server connect hook
		 * This function lets modules know that another server has connected to the network.
		 * @param newServerName The name of the new server
		 */
		void callServerConnectHook(std::string newServerName);
		
		/** Call server quit hook
		 * This function lets modules know that a server has quit from the network.
		 * @param quitServerName The name of the server that quit
		 * @param reason The quit reason
		 */
		void callServerQuitHook(std::string quitServerName, std::string reason);
		
		/** Call other data hook
		 * This function lets modules know that you have received a line from the server that cannot be handled by other API functions.
		 * @param client The identifier of the client who received the line (may be an empty string if not to a specific client)
		 * @param lineTokens The line, split into IRC tokens
		 */
		void callOtherDataHook(std::string client, std::vector<std::string> lineTokens);
		
		/** Call outgoing channel message hook
		 * This function lets modules know that you are ready to send out a channel message.
		 * @param client The identifier of the client from which the message will be sent
		 * @param channel The channel to which the message will be sent
		 * @param status The status char to which the message will be sent, e.g. '@' if it will be sent to @#channel; it should be a blank
		 * space (' ') if it is to be sent to the entire channel
		 * @param message The message to send
		 */
		void callChanMsgOutHook(std::string client, std::string channel, char status, std::string message);
		
		/** Call sent channel message hook
		 * This function lets modules know that you have sent a message to a channel.
		 * @param client The identifier of the client from which the message was sent
		 * @param channel The channel to which the message was sent
		 * @param status The status char to which the message was sent, e.g. '@' if it was sent to @#channel; it should be a blank space (' ')
		 * if it was sent to the entire channel
		 * @param message The message sent
		 */
		void callChanMsgSendHook(std::string client, std::string channel, char status, std::string message);
		
		/** Call outgoing user message hook
		 * This function lets modules know that you are ready to send out a user message.
		 * @param client The identifier of the client from which the message will be sent
		 * @param nick The nick of the user to whom the message will be sent
		 * @param message The message to send
		 */
		void callUserMsgOutHook(std::string client, std::string nick, std::string message);
		
		/** Call sent user message hook
		 * This function lets modules know that you have sent a message to a user.
		 * @param client The identifier of the client from which the message was sent
		 * @param nick The nick of the user to whom the message was sent
		 * @param message The message sent
		 */
		void callUserMsgSendHook(std::string client, std::string nick, std::string message);
		
		/** Call outgoing channel notice hook
		 * This function lets modules know that you are ready to send out a channel notice.
		 * @param client The identifier of the client from which the notice will be sent
		 * @param channel The channel to which the notice will be sent
		 * @param status The status char to which the message will be sent, e.g. '@' if it will be sent to @#channel; it should be a blank
		 * space (' ') if it will be sent to the entire channel
		 * @param message The message to send
		 */
		void callChanNoticeOutHook(std::string client, std::string channel, char status, std::string message);
		
		/** Call sent channel notice hook
		 * This function lets modules know that you have sent out a channel notice.
		 * @param client The identifier of the client from which the notice was sent
		 * @param channel The channel to which the notice was sent
		 * @param status The status char to which the notice was sent, e.g. '@' if it was sent to @#channel; it should be a blank space (' ')
		 * if it was sent to the entire channel
		 * @param message The message sent
		 */
		void callChanNoticeSendHook(std::string client, std::string channel, char status, std::string message);
		
		/** Call outgoing user notice hook
		 * This function lets modules know that you are ready to send out a user notice.
		 * @param client The identifier of the client from which the notice will be sent
		 * @param nick The nick of the user to whom the notice will be sent
		 * @param message The message to send
		 */
		void callUserNoticeOutHook(std::string client, std::string nick, std::string message);
		
		/** Call sent user notice hook
		 * This function lets modules know that you have sent out a user notice.
		 * @param client The identifier of the client that sent the notice
		 * @param nick The nick of the user to whom the notice was sent
		 * @param message The message sent
		 */
		void callUserNoticeSendHook(std::string client, std::string nick, std::string message);
		
		/** Call outgoing channel CTCP hook
		 * This function lets modules know that you are ready to send out a channel CTCP request.
		 * @param client The identifier of the client that will send the CTCP request
		 * @param channel The channel to which the CTCP request will be sent
		 * @param status The status char to which the CTCP request will be sent, e.g. '@' if it will be sent to @#channel; it should be a blank
		 * space (' ') if it will be sent to the entire channel
		 * @param ctcp The CTCP type and any associated parameters
		 */
		void callChanCTCPOutHook(std::string client, std::string channel, char status, std::string ctcp);
		
		/** Call sent channel CTCP hook
		 * This function lets modules know that you have sent out a channel CTCP request.
		 * @param client The identifier of the client that sent the CTCP request
		 * @param channel The channel to which the CTCP request was sent
		 * @param status The status char to which the CTCP request was sent, e.g. '@' if it was sent to @#channel; it should be a blank space
		 * (' ') if it was sent to the entire channel
		 * @param ctcp The CTCP type and any associated parameters
		 */
		void callChanCTCPSendHook(std::string client, std::string channel, char status, std::string ctcp);
		
		/** Call outgoing user CTCP hook
		 * This function lets modules know that you are ready to send out a user CTCP request.
		 * @param client The identifier of the client that will send the CTCP request
		 * @param nick The nick of the user to whom the CTCP request will be sent
		 * @param ctcp The CTCP type and any associated parameters
		 */
		void callUserCTCPOutHook(std::string client, std::string nick, std::string ctcp);
		
		/** Call sent user CTCP hook
		 * This function lets modules know that you have sent out a user CTCP request.
		 * @param client The identifier of the client that sent the CTCP request
		 * @param nick The nick of the user to whom the CTCP request was sent
		 * @param ctcp The CTCP type and any associated parameters
		 */
		void callUserCTCPSendHook(std::string client, std::string nick, std::string ctcp);
		
		/** Call outgoing channel CTCP reply hook
		 * This function lets modules know that you are ready to send out a channel CTCP reply.
		 * @param client The identifier of the client that will send the CTCP reply
		 * @param channel The channel to which the CTCP reply will be sent
		 * @param status The status char to which the CTCP reply will be sent, e.g. '@' if it will be sent to @#channel; it should be a blank
		 * space (' ') if it will be sent to the entire channel
		 * @param ctcp The CTCP type and any associated data
		 */
		void callChanCTCPReplyOutHook(std::string client, std::string channel, char status, std::string ctcp);
		
		/** Call sent channel CTCP reply hook
		 * This function lets modules know that you have sent out a channel CTCP reply.
		 * @param client The identifier of the client that sent the CTCP reply
		 * @param channel The channel to which the CTCP reply was sent
		 * @param status The status char to which the CTCP reply was sent, e.g. '@' if it was sent to @#channel; it should be a blank space (' ')
		 * if it was sent to the entire channel
		 * @param ctcp The CTCP type and any associated data
		 */
		void callChanCTCPReplySendHook(std::string client, std::string channel, char status, std::string ctcp);
		
		/** Call outgoing user CTCP reply hook
		 * This function lets modules know that you are ready to send out a user CTCP reply
		 * @param client The identifier of the client that will send the CTCP reply
		 * @param nick The nick of the user to whom the CTCP reply will be sent
		 * @param ctcp The CTCP type and any associated data
		 */
		void callUserCTCPReplyOutHook(std::string client, std::string nick, std::string ctcp);
		
		/** Call sent user CTCP reply hook
		 * This function lets modules know that you have sent out a user CTCP reply
		 * @param client The identifier of the client that sent the CTCP reply
		 * @param nick The nick of the user to whom the CTCP reply was sent
		 * @param ctcp The CTCP type and any associated data
		 */
		void callUserCTCPReplySendHook(std::string client, std::string nick, std::string ctcp);
	private:
		/// A pointer to the bot base which allows communication with bot modules
		Base* bot;
};

#include "base.h"