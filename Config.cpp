/*
* Copyright (c) 2013-2016, The PurpleI2P Project
*
* This file is part of Purple i2pd project and licensed under BSD3
*
* See full license text in LICENSE file at top of project tree
*/

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "Config.h"
#include "version.h"

using namespace boost::program_options;

namespace i2p {
namespace config {
  options_description m_OptionsDesc;
  variables_map       m_Options;

  void Init() {
    bool nat = true;
#ifdef MESHNET
    nat = false;
#endif

    options_description general("General options");
    general.add_options()
      ("help",     "Show this message")
      ("conf",      value<std::string>()->default_value(""),     "Path to main i2pd config file (default: try ~/.i2pd/i2pd.conf or /var/lib/i2pd/i2pd.conf)")
      ("tunconf",   value<std::string>()->default_value(""),     "Path to config with tunnels list and options (default: try ~/.i2pd/tunnels.conf or /var/lib/i2pd/tunnels.conf)")
      ("pidfile",   value<std::string>()->default_value(""),     "Path to pidfile (default: ~/i2pd/i2pd.pid or /var/lib/i2pd/i2pd.pid)")
      ("log",       value<std::string>()->default_value(""),     "Logs destination: stdout, file, syslog (stdout if not set)")
      ("logfile",   value<std::string>()->default_value(""),     "Path to logfile (stdout if not set, autodetect if daemon)")
      ("loglevel",  value<std::string>()->default_value("info"), "Set the minimal level of log messages (debug, info, warn, error)")
	  ("family",    value<std::string>()->default_value(""),     "Specify a family, router belongs to")
	  ("datadir",   value<std::string>()->default_value(""),     "Path to storage of i2pd data (RI, keys, peer profiles, ...)")
      ("host",      value<std::string>()->default_value("0.0.0.0"),     "External IP")
      ("ifname",    value<std::string>()->default_value(""), "network interface to bind to")
      ("nat",       value<bool>()->zero_tokens()->default_value(nat), "should we assume we are behind NAT?")
      ("port",      value<uint16_t>()->default_value(0),                "Port to listen for incoming connections (default: auto)")
      ("ipv4",      value<bool>()->zero_tokens()->default_value(true),  "Enable communication through ipv4")
      ("ipv6",      value<bool>()->zero_tokens()->default_value(false), "Enable communication through ipv6")
      ("daemon",    value<bool>()->zero_tokens()->default_value(false), "Router will go to background after start")
      ("service",   value<bool>()->zero_tokens()->default_value(false), "Router will use system folders like '/var/lib/i2pd'")
      ("notransit", value<bool>()->zero_tokens()->default_value(false), "Router will not accept transit tunnels at startup")
      ("floodfill", value<bool>()->zero_tokens()->default_value(false), "Router will be floodfill")
      ("bandwidth", value<std::string>()->default_value(""), "Bandwidth limit: integer in kbps or letters: L (32), O (256), P (2048), X (>9000)")
      ("ntcp", value<bool>()->zero_tokens()->default_value(true), "enable ntcp transport")
      ("ssu", value<bool>()->zero_tokens()->default_value(true), "enable ssu transport")
#ifdef _WIN32
      ("svcctl",    value<std::string>()->default_value(""),     "Windows service management ('install' or 'remove')")
      ("insomnia", value<bool>()->zero_tokens()->default_value(false), "Prevent system from sleeping")
      ("close", value<std::string>()->default_value("ask"), "Action on close: minimize, exit, ask") // TODO: add custom validator or something
#endif
      ;

    options_description limits("Limits options");
    limits.add_options()
      ("limits.transittunnels",   value<uint16_t>()->default_value(2500), "Maximum active transit sessions (default:2500)")
      ;

    options_description httpserver("HTTP Server options");
    httpserver.add_options()
      ("http.enabled",        value<bool>()->default_value(true),               "Enable or disable webconsole")
      ("http.address",        value<std::string>()->default_value("127.0.0.1"), "Webconsole listen address")
      ("http.port",           value<uint16_t>()->default_value(7070),           "Webconsole listen port")
      ("http.auth",           value<bool>()->default_value(false),              "Enable Basic HTTP auth for webconsole")
      ("http.user",           value<std::string>()->default_value("i2pd"),      "Username for basic auth")
      ("http.pass",           value<std::string>()->default_value(""),          "Password for basic auth (default: random, see logs)")
      ;

    options_description httpproxy("HTTP Proxy options");
    httpproxy.add_options()
      ("httpproxy.enabled",   value<bool>()->default_value(true),                         "Enable or disable HTTP Proxy")
      ("httpproxy.address",   value<std::string>()->default_value("127.0.0.1"),           "HTTP Proxy listen address")
      ("httpproxy.port",      value<uint16_t>()->default_value(4444),                     "HTTP Proxy listen port")
      ("httpproxy.keys",      value<std::string>()->default_value(""),  "File to persist HTTP Proxy keys")
      ;

    options_description socksproxy("SOCKS Proxy options");
    socksproxy.add_options()
      ("socksproxy.enabled",  value<bool>()->default_value(true),                         "Enable or disable SOCKS Proxy")
      ("socksproxy.address",  value<std::string>()->default_value("127.0.0.1"),           "SOCKS Proxy listen address")
      ("socksproxy.port",     value<uint16_t>()->default_value(4447),                     "SOCKS Proxy listen port")
      ("socksproxy.keys",     value<std::string>()->default_value(""), "File to persist SOCKS Proxy keys")
      ("socksproxy.outproxy", value<std::string>()->default_value("127.0.0.1"), "Upstream outproxy address for SOCKS Proxy")
      ("socksproxy.outproxyport", value<uint16_t>()->default_value(9050), "Upstream outproxy port for SOCKS Proxy")
      ;

    options_description sam("SAM bridge options");
    sam.add_options()
      ("sam.enabled",         value<bool>()->default_value(false),                        "Enable or disable SAM Application bridge")
      ("sam.address",         value<std::string>()->default_value("127.0.0.1"),           "SAM listen address")
      ("sam.port",            value<uint16_t>()->default_value(7656),                     "SAM listen port")
      ;

    options_description bob("BOB options");
    bob.add_options()
      ("bob.enabled",         value<bool>()->default_value(false),                        "Enable or disable BOB command channel")
      ("bob.address",         value<std::string>()->default_value("127.0.0.1"),           "BOB listen address")
      ("bob.port",            value<uint16_t>()->default_value(2827),                     "BOB listen port")
      ;

	options_description i2cp("I2CP options");
    i2cp.add_options()
      ("i2cp.enabled",        value<bool>()->default_value(false),                        "Enable or disable I2CP")
      ("i2cp.address",        value<std::string>()->default_value("127.0.0.1"),           "I2CP listen address")
      ("i2cp.port",            value<uint16_t>()->default_value(7654),                     "I2CP listen port")
      ;

    options_description i2pcontrol("I2PControl options");
    i2pcontrol.add_options()
      ("i2pcontrol.enabled",  value<bool>()->default_value(false),                        "Enable or disable I2P Control Protocol")
      ("i2pcontrol.address",  value<std::string>()->default_value("127.0.0.1"),           "I2PCP listen address")
      ("i2pcontrol.port",     value<uint16_t>()->default_value(7650),                     "I2PCP listen port")
      ("i2pcontrol.password", value<std::string>()->default_value("itoopie"),             "I2PCP access password")
      ("i2pcontrol.cert",     value<std::string>()->default_value("i2pcontrol.crt.pem"),  "I2PCP connection cerificate")
      ("i2pcontrol.key",      value<std::string>()->default_value("i2pcontrol.key.pem"),  "I2PCP connection cerificate key")
      ;

	bool upnp_default = false;
#if (defined(USE_UPNP) && ((defined(WIN32) && defined(USE_WIN32_APP)) || defined(ANDROID)))
	upnp_default = true; // enable UPNP for windows GUI and android by default	
#endif
  	options_description upnp("UPnP options");
  	upnp.add_options()
    ("upnp.enabled",  value<bool>()->default_value(upnp_default),             "Enable or disable UPnP: automatic port forwarding")
    ;

	options_description precomputation("Precomputation options");
	precomputation.add_options()  
	  ("precomputation.elgamal",  
#if defined(__x86_64__)	   
	   value<bool>()->default_value(false),   
#else
	   value<bool>()->default_value(true),  
#endif	   
	   "Enable or disable elgamal precomputation table")
	  ;

  options_description trust("Trust options");
  trust.add_options()
    ("trust.enabled", value<bool>()->default_value(false), "enable explicit trust options")
    ("trust.family", value<std::string>()->default_value(""), "Router Familiy to trust for first hops")
    ("trust.hidden", value<bool>()->default_value(false), "should we hide our router from other routers?");
  
    m_OptionsDesc
      .add(general)
	  .add(limits)	
      .add(httpserver)
      .add(httpproxy)
      .add(socksproxy)
      .add(sam)
      .add(bob)
	  .add(i2cp)	
      .add(i2pcontrol)
      .add(upnp)
	  .add(precomputation)
      .add(trust)
      ;
  }

  void ParseCmdline(int argc, char* argv[]) {
    try {
      auto style = boost::program_options::command_line_style::unix_style
                 | boost::program_options::command_line_style::allow_long_disguise;
      style &=   ~ boost::program_options::command_line_style::allow_guessing;
      store(parse_command_line(argc, argv, m_OptionsDesc, style), m_Options);
    } catch (boost::program_options::error& e) {
      std::cerr << "args: " << e.what() << std::endl;
      exit(EXIT_FAILURE);
    }

    if (m_Options.count("help") || m_Options.count("h")) {
      std::cout << "i2pd version " << I2PD_VERSION << " (" << I2P_VERSION << ")" << std::endl;
      std::cout << m_OptionsDesc;
      exit(EXIT_SUCCESS);
    }
  }

  void ParseConfig(const std::string& path) {
    if (path == "") return;

    std::ifstream config(path, std::ios::in);

    if (!config.is_open()) 
	{
      std::cerr << "missing/unreadable config file: " << path << std::endl;
      exit(EXIT_FAILURE);
    }

    try 
	{
		store(boost::program_options::parse_config_file(config, m_OptionsDesc), m_Options);
    } 
	catch (boost::program_options::error& e) 
	{
      std::cerr << e.what() << std::endl;
      exit(EXIT_FAILURE);
    };
  }

  void Finalize() {
    notify(m_Options);
  }

  bool IsDefault(const char *name) {
    if (!m_Options.count(name))
      throw "try to check non-existent option";

    if (m_Options[name].defaulted())
      return true;
    return false;
  }
} // namespace config
} // namespace i2p
