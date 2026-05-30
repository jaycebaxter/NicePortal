/**
 * .
 */
#include "ConfigParser.h"
#include "LineReader.h"

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <cstdint>
#include <LittleFS.h>


// network settings
const IPAddress NET_IP(192, 168, 1, 1);  // address of web interface
const IPAddress NET_SUBNET(255, 255, 255, 0);

// server settings
const uint8_t DNS_PORT = 53;
const uint8_t HTTP_PORT = 80;
const char *CACHE_CONTROL_HEADER = "max-age=86400";

// paths
const char *WEBROOT_PATH = "/www/";
const char *CONFIG_PATH = "/settings.conf";
const char *GUESTS_PATH = "/guests.log";

// default settings
const char *DEFAULT_AP_SSID = "NicePortal";
const char *DEFAULT_AP_PASSWORD = NULL;
const char *DEFAULT_PORTAL_DOMAIN = "niceportal.lan";

FS &fileSystem = LittleFS;
ConfigParser configParser;
const char *portalDomain;
DNSServer dnsServer;
ESP8266WebServer webServer;

void setup() {
    system_update_cpu_freq(160);
    // start SD card filesystem
    fileSystem.begin();

    // load settings from config file
    File configFile = fileSystem.open(CONFIG_PATH, "r");
    if (configFile) {
        configParser.parse(configFile);
        configFile.close();
    }
    portalDomain = configParser.get("portal_domain", DEFAULT_PORTAL_DOMAIN),

    // register handlers
    webServer.on(GUESTS_PATH, HTTP_GET,  handleGetGuests);
    webServer.on(GUESTS_PATH, HTTP_POST, handlePostGuests);
    webServer.serveStatic("/", fileSystem, WEBROOT_PATH, CACHE_CONTROL_HEADER);
    webServer.onNotFound(handleNotFound);
    webServer.begin(HTTP_PORT);

    // use local DNS server to implement captive portal
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", NET_IP);

    // host AP for clients to connect to
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(NET_IP, NET_IP, NET_SUBNET);
    WiFi.softAP(
        configParser.get("ap_ssid",     DEFAULT_AP_SSID),
        configParser.get("ap_password", DEFAULT_AP_PASSWORD));
}

void loop() {
    dnsServer.processNextRequest();
    webServer.handleClient();
}

void handleNotFound() {
    String redirectUrl = "http://";
    redirectUrl += portalDomain;
    redirectUrl += "/";

    // redirect to portal url if requesting a different domain
    if (!webServer.hostHeader().equals(portalDomain)) {
        webServer.sendHeader("Location", redirectUrl);
        webServer.send(302);
        return;
    }

    // default to generic 404 message
    String error = "File ";
    error += webServer.uri();
    error += " not found.";
    webServer.send(404, "text/plain", error);
}

void handleGetGuests() {
    // serve empty file if log hasn't been created yet
    File guestsFile = fileSystem.open(GUESTS_PATH, "r");
    if (!guestsFile) {
        webServer.send(200, "text/plain", "");
        return;
    }

    // serve file contents
    webServer.streamFile(guestsFile, "text/plain");
    guestsFile.close();
}

void handlePostGuests() {
    // reject requests without a body
    if (!webServer.hasArg("plain")) {
        webServer.send(400, "text/plain",
            "Request body is required to add record.");
        return;
    }

    // load settings from config file
    File guestsFile = fileSystem.open(GUESTS_PATH, "a");
    if (!guestsFile) {
        webServer.send(500, "text/plain", "Could not open database.");
        return;
    }

    // write non-empty lines to database
    String body = webServer.arg("plain");
    StringStream stringStream(body);
    LineReader lineReader(stringStream);
    while (lineReader.hasMore()) {
        String line = lineReader.readLine();
        if (line.length() > 0) {
            guestsFile.print(line);
            guestsFile.print('\n');
        }
    }

    // write record separator
    guestsFile.print('\n');
    guestsFile.close();

    webServer.send(200);
}
