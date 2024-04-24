#ifndef _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_

#define SMEXT_CONF_NAME			"SourceMod JSON Extension"
#define SMEXT_CONF_DESCRIPTION	"Provide JSON Native"
#define SMEXT_CONF_VERSION		"1.0.5"
#define SMEXT_CONF_AUTHOR		"ProjectSky"
#define SMEXT_CONF_URL			"https://github.com/ProjectSky/sm-ext-json"
#define SMEXT_CONF_LOGTAG		"json"
#define SMEXT_CONF_LICENSE		"GPL"
#define SMEXT_CONF_DATESTRING	__DATE__

#define SMEXT_LINK(name) SDKExtension *g_pExtensionIface = name;

#define SMEXT_ENABLE_HANDLESYS

#endif // _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_
