#include <Arduino.h>

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <time.h>

#include <HTTPClient.h>

#include "tinyxml2.h"

#define WIFI_SSID "iotec guest"//"HK" //"iotec guest"
#define WIFI_password "weilefehlengestopptbenutzt"//"HK_AP_333" // "weilefehlengestopptbenutzt"

#define ApiToken "Bearer 298d2245312152f3b69c89f3310f6352"

#define departureStation_Ibbenbueren "8003036"
#define departureStation_OsnabrueckAltstadt "8004689"

GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4));

WiFiMulti wifiMulti;

const char* ntpServer = "europe.pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
int second;
int minute;
int hour;
int day;
int month;
int year;
int weekday;
long current;
struct tm timeinfo;
int actHour = -1;
  /*
struct tm
{
int    tm_sec;   //   Seconds [0,60]. 
int    tm_min;   //   Minutes [0,59]. 
int    tm_hour;  //   Hour [0,23]. 
int    tm_mday;  //   Day of month [1,31]. 
int    tm_mon;   //   Month of year [0,11]. 
int    tm_year;  //   Years since 1900. 
int    tm_wday;  //   Day of week [0,6] (Sunday =0). 
int    tm_yday;  //   Day of year [0,365]. 
int    tm_isdst; //   Daylight Savings flag. 
}
 */  

HTTPClient http;

// https certificate from HTTPClient example
const char* ca = 
"-----BEGIN CERTIFICATE-----\n" 
"MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n"   
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n"   
"DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n"   
"SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n"   
"GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n"   
"AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n"   
"q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n"   
"SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n"   
"Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n"   
"a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n"   
"/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n"   
"AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n"   
"CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n"  
"c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n"   
"VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n"  
"ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n"   
"MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n"   
"Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n"   
"AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n"   
"uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n"   
"wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n"   
"X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n"   
"PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n"   
"KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n"   
"-----END CERTIFICATE-----\n";

static char departureStation[] = departureStation_OsnabrueckAltstadt;

struct train
{
    char   id[50] = "0";//const char *id;
    char   company[23] = "unknown";//const char *company;
    char   line[23] = "unknown";//const char *line;
    char   nextStop[23] = "unknown";
    bool   validTrain = false;
    char   plannedDepartureYY[3] = {'\0'};
    char   plannedDepartureMM[3] = {'\0'};
    char   plannedDepartureDD[3] = {'\0'};
    char   plannedDepartureHH[3] = {'\0'};
    char   plannedDeparturemm[3] = {'\0'};
    char   actDepartureYY[3] = {'\0'};
    char   actDepartureMM[3] = {'\0'};
    char   actDepartureDD[3] = {'\0'};
    char   actDepartureHH[3] = {'\0'};
    char   actDeparturemm[3] = {'\0'};
    int    delayDeparture = 0;
    
};
train trains[10];

int plannedArrayCounter = 0;

String tmpString;

bool connectToWifi(const char *ssid, const char *password)
{
    wifiMulti.addAP(ssid, password);

    // wait for WiFi connection
    if((wifiMulti.run() == WL_CONNECTED)) 
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool getNtpTime()
{
    //init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    if(!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return false;
    }

    //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    second = timeinfo.tm_sec;
    minute = timeinfo.tm_min;
    hour = timeinfo.tm_hour;
    day = timeinfo.tm_mday;
    month = timeinfo.tm_mon + 1;
    year = timeinfo.tm_year + 1900;
    weekday = timeinfo.tm_wday +1;

    return true;
}

void httpsBegin(String httpsURL)
{
    Serial.print(httpsURL);
    // allow reuse (if server supports it)
    http.setReuse(true);

    // configure traged server and url
    http.begin(httpsURL, ca); //HTTPS
    //http.begin("http://api.deutschebahn.com/timetables/v1/station/Ibb"); //HTTP

    // add headers for API
    http.addHeader("Accept", "application/xml");
    http.addHeader("Authorization", ApiToken); 
}

bool httpsGet()
{

    // start connection and send HTTP header (httpCode will be negative on error)
    int httpCode = http.GET();
    if(httpCode == HTTP_CODE_OK) 
    {
        return true;
    }
    else
    {
        Serial.printf("\n! httpsGet() error %d: %s\n", httpCode, http.errorToString(httpCode).c_str());
        return false;
    }  
}

bool getPlannedTimetable(char *stationID, int departureHour)
{
    Serial.print("Get planned timetable for departure station ");
    Serial.println(stationID);

    Serial.print("begin https connection ");

    tmpString = "https://api.deutschebahn.com/timetables/v1/plan/";
    tmpString += stationID;
    tmpString += "/";
    tmpString += String(year - 2000);
    if (month < 10)
    {
        tmpString += "0";
    }
    tmpString += String(month);
    if (day < 10)
    {
        tmpString += "0";
    }
    tmpString += String(day);
    tmpString += "/";
    if (departureHour < 10)
    {
        tmpString += "0";
    }
    tmpString += String(departureHour);

    httpsBegin(tmpString);
    Serial.print("... ");

    Serial.print("done\n");

    Serial.print("https get planned timetable for departureHour... ");

    if(httpsGet() == true)
    {
        Serial.print("done\n");
    }
    else
    {
        Serial.print("failed\n");
        return false;
    }

    return true;
}

bool httpsProcessPlannedTimetable()
{
    String payload = http.getString();
    //Serial.printf("received string length: %d\n", payload.length());
    //Serial.println(payload);

    const char *xml = payload.c_str();
    /*int i = 0;
    while (*(xml+i) != '\0')
    {
        i++;
    }
    Serial.printf("const char xml length: %d\n", i);*/
    //Serial.println(xml);

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError error;
    error = doc.Parse(xml);
    if(error != 0)
    {
        Serial.printf("\n! error: parsing xml\n");
        return false;
    }

    tinyxml2::XMLElement* element = NULL;

    const char *pointerToXmlContent;

    char tmpNextStop[23] = "unknown";

    int arraySize = sizeof(trains)/sizeof(*trains);
    Serial.printf( "   max number of trains: %d\n", arraySize);

    element = doc.FirstChildElement("timetable")->FirstChildElement("s");
    if(element == NULL)
    {
        Serial.printf("\n! error: xml access FirstChildElement(\"timetable\")->FirstChildElement(\"s\")\n");
        return false;
    }
Serial.printf("   searching for trains...\n");
    while ((element != NULL) && (plannedArrayCounter < arraySize))
    {
        Serial.printf("   searching for train %d...\n", plannedArrayCounter);
        if((element != NULL) && (plannedArrayCounter < arraySize))
        {
            error = element->FirstChildElement( "dp" )->QueryStringAttribute( "ppth", &pointerToXmlContent);
            if(error != 0)
            {
                Serial.printf("\n! error: reading xml attribute\n");
                return false;
            }
            strncpy(tmpNextStop, pointerToXmlContent, 22);
            Serial.printf("   found train with nextStop %s\n", tmpNextStop);
            if(strcmp(tmpNextStop, "Ibbenbüren-Laggenbeck") == 0)
            {
                strncpy(trains[plannedArrayCounter].nextStop, pointerToXmlContent, 22);

                error = element->QueryStringAttribute( "id", &pointerToXmlContent);
                if(error != 0)
                {
                    Serial.printf("\n! error: reading xml attribute\n");
                    return false;
                }
                strncpy(trains[plannedArrayCounter].id, pointerToXmlContent, 48);
                //trains[plannedArrayCounter].id[49] = '\0';
                Serial.printf("      found train[%d] with id %s", plannedArrayCounter, trains[plannedArrayCounter].id);

                error = element->FirstChildElement( "tl" )->QueryStringAttribute( "c", &pointerToXmlContent);
                if(error != 0)

                {
                    Serial.printf("\n! error: reading xml attribute\n");
                    return false;
                }
                strncpy(trains[plannedArrayCounter].company, pointerToXmlContent, 21);

                error = element->FirstChildElement( "dp" )->QueryStringAttribute( "l", &pointerToXmlContent);
                if(error != 0)
                {
                    Serial.printf("\n! error: reading xml attribute\n");
                    return false;
                }
                strncpy(trains[plannedArrayCounter].line, pointerToXmlContent, 21);

                /*error = element->FirstChildElement( "dp" )->QueryStringAttribute( "ppth", &pointerToXmlContent);
                if(error != 0)
                {
                    Serial.printf("\n! error: reading xml attribute\n");
                    return false;
                }
                strncpy(trains[plannedArrayCounter].nextStop, pointerToXmlContent, 22);*/

                error = element->FirstChildElement( "dp" )->QueryStringAttribute( "pt", &pointerToXmlContent);
                if(error != 0)
                {
                    Serial.printf("\n! error: reading xml attribute\n");
                    return false;
                }
                strncpy(trains[plannedArrayCounter].plannedDepartureYY, pointerToXmlContent, 2);
                //trains[plannedArrayCounter].plannedDepartureYY[2] = '\0';

                strncpy(trains[plannedArrayCounter].plannedDepartureMM, pointerToXmlContent + 2, 2);
                //trains[plannedArrayCounter].plannedDepartureMM[2] = '\0';

                strncpy(trains[plannedArrayCounter].plannedDepartureDD, pointerToXmlContent + 4, 2);
                //trains[plannedArrayCounter].plannedDepartureDD[2] = '\0';

                strncpy(trains[plannedArrayCounter].plannedDepartureHH, pointerToXmlContent + 6, 2);
                //trains[plannedArrayCounter].plannedDepartureHH[2] = '\0';

                strncpy(trains[plannedArrayCounter].plannedDeparturemm, pointerToXmlContent + 8, 2);
                //trains[plannedArrayCounter].plannedDeparturemm[2] = '\0';

                Serial.printf(", planned departure: %s:%s\n", trains[plannedArrayCounter].plannedDepartureHH, trains[plannedArrayCounter].plannedDeparturemm);

                plannedArrayCounter++;
            }
            else
            {
                Serial.printf("      train is not heading to Ibbenbüren-Laggenbeck\n");
            }
            
        }

        element = element->NextSiblingElement("s");
        if(element == NULL)
        {
            Serial.printf("   all trains found\n");
            //return false;
        }
        else
        {
            Serial.printf("   get data for next train\n");//Serial.printf("NextSiblingElement(\"s\")\n");
        }
    }

    Serial.printf( "   trains found: %d\n", plannedArrayCounter);

    return true;
}

bool getDeviatedTimetable(char *stationID)
{
    Serial.print("Get deviated timetable for departure station ");
    Serial.println(stationID);

    Serial.print("begin https connection for deviated timetable... ");

    tmpString = "https://api.deutschebahn.com/timetables/v1/fchg/";
    tmpString += departureStation;
    
    httpsBegin(tmpString);

    Serial.print("... ");

    Serial.print("done\n");

    Serial.print("https get deviated timetable... ");

    if(httpsGet() == true)
    {
        Serial.print("done\n");
    }
    else
    {
        Serial.print("failed get devi\n");
        return false;
    }

    return true;
}

bool httpsProcessDeviatedTimetable()
{
    String payload = http.getString();
    //Serial.printf("received string length: %d\n", payload.length());
    //Serial.println(payload);

    const char *xml = payload.c_str();
    /*int i = 0;
    while (*(xml+i) != '\0')
    {
        i++;
    }
    Serial.printf("const char xml length: %d\n", i);
    Serial.println(xml);*/

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError error;
    error = doc.Parse(xml);
    if(error != 0)
    {
        Serial.printf("\n! error: parsing xml\n");
        return false;
    }

    tinyxml2::XMLElement* element = NULL;

    int deviatedArrayCounter = 0;

    const char *pointerToXmlContent;

    int i = 0;

    int arraySize = sizeof(trains)/sizeof(*trains);
    Serial.printf( "   max number of delays: %d\n", arraySize);

    element = doc.FirstChildElement("timetable")->FirstChildElement("s");
    if(element == NULL)
    {
        Serial.printf("\n! error: xml access FirstChildElement(\"timetable\")->FirstChildElement(\"s\")\n");
        return false;
    }

    while ((element != NULL) && (deviatedArrayCounter < arraySize))
    {
        if((element != NULL) && (deviatedArrayCounter < arraySize))
        {
            i++;
            error = element->QueryStringAttribute( "id", &pointerToXmlContent);
            if(error != 0)
            {
                Serial.printf("\n! error: reading xml attribute\n");
                return false;
            }

            Serial.printf("   found delay with train id %s and compare it with train[%d] id", pointerToXmlContent, deviatedArrayCounter);

            if(strcmp(trains[deviatedArrayCounter].id, pointerToXmlContent) == 0)
            {
                error = element->FirstChildElement( "dp" )->QueryStringAttribute( "ct", &pointerToXmlContent);
                if(error != 0)
                {
                    Serial.printf("\n! error: reading xml attribute\n");
                    return false;
                }
                Serial.printf(" -> delayed time: %s", pointerToXmlContent);

                char   tmpDepartureYY[3] = {'\0'};
                char   tmpDepartureMM[3] = {'\0'};
                char   tmptDepartureDD[3] = {'\0'};
                char   tmpDepartureHH[3] = {'\0'};
                char   tmpDeparturemm[3] = {'\0'};

                strncpy(tmpDepartureHH, pointerToXmlContent + 6, 2);
                //trains[deviatedArrayCounter].actDepartureHH[2] = '\0';

                strncpy(tmpDeparturemm, pointerToXmlContent + 8, 2);
                //trains[deviatedArrayCounter].actDeparturemm[2] = '\0';

                //Serial.printf("delay: %d - %d = %d\n", atoi(trains[deviatedArrayCounter].actDeparturemm), atoi(trains[deviatedArrayCounter].plannedDeparturemm), (atoi(trains[deviatedArrayCounter].actDeparturemm) - atoi(trains[deviatedArrayCounter].plannedDeparturemm)));
                trains[deviatedArrayCounter].delayDeparture = (((atoi(tmpDepartureHH) - atoi(trains[deviatedArrayCounter].plannedDepartureHH)) * 60) + (atoi(tmpDeparturemm) - atoi(trains[deviatedArrayCounter].plannedDeparturemm)));

                if(trains[deviatedArrayCounter].delayDeparture > 0)
                {
                    strncpy(trains[deviatedArrayCounter].actDepartureYY, pointerToXmlContent, 2);
                    //trains[deviatedArrayCounter].actDepartureYY[2] = '\0';

                    strncpy(trains[deviatedArrayCounter].actDepartureMM, pointerToXmlContent + 2, 2);
                    //trains[deviatedArrayCounter].actDepartureMM[2] = '\0';

                    strncpy(trains[deviatedArrayCounter].actDepartureDD, pointerToXmlContent + 4, 2);
                    //trains[deviatedArrayCounter].actDepartureDD[2] = '\0';

                    strncpy(trains[deviatedArrayCounter].actDepartureHH, pointerToXmlContent + 6, 2);
                    //trains[deviatedArrayCounter].actDepartureHH[2] = '\0';

                    strncpy(trains[deviatedArrayCounter].actDeparturemm, pointerToXmlContent + 8, 2);
                    //trains[deviatedArrayCounter].actDeparturemm[2] = '\0';
                }

                deviatedArrayCounter++;
            }

            Serial.println();
        }

        element = element->NextSiblingElement("s");
        if(element == NULL)
        {
            Serial.printf("   all delays found\n");
            //return false;
        }
        else
        {
            Serial.printf("   get data for next delay\n");//Serial.printf("NextSiblingElement(\"s\")\n");
        }
    }

    Serial.printf( "   delay for trains found: %d\n", deviatedArrayCounter);

    return true;
}

void printTrains()
{
    char tmpString[200];
    char BTbuffer[1024];
    if(departureStation == "departureStation_Ibbenbüren")
    {
        sprintf(tmpString, "--- trainDelay ---\nIbbenbueren -> Osnabrueck\nlast update: %d:%d:%d\n", hour, minute, second);
    }
    else
    {
        sprintf(tmpString, "--- trainDelay ---\nOsnabrueck -> Ibbenbueren\nlast update: %d:%d:%d\n", hour, minute, second);
    }

    strcpy(BTbuffer, tmpString);

    int arraySize = sizeof(trains)/sizeof(*trains);
    Serial.printf( "   max number of trains: %d\n", arraySize);

    for (int counter = 0; counter < arraySize; counter++)
    {
        if(strcmp(trains[counter].id, "0") != 0)
        {
            if(trains[counter].delayDeparture == 0)
            {
                strncpy(trains[counter].actDepartureYY, trains[counter].plannedDepartureYY, 2);
                strncpy(trains[counter].actDepartureMM, trains[counter].plannedDepartureMM, 2);
                strncpy(trains[counter].actDepartureDD, trains[counter].plannedDepartureDD, 2);
                strncpy(trains[counter].actDepartureHH, trains[counter].plannedDepartureHH, 2);
                strncpy(trains[counter].actDeparturemm, trains[counter].plannedDeparturemm, 2);
            }

            Serial.printf( "   train[%d]\n", counter);
            Serial.printf( "      id:                 %s\n", trains[counter].id);
            Serial.printf( "      company:            %s\n", trains[counter].company);
            Serial.printf( "      line:               %s\n", trains[counter].line);
            Serial.printf( "      nextStop:           %s\n", trains[counter].nextStop);
            if(strcmp(trains[counter].nextStop, "Ibbenbüren-Laggenbeck") == 0)
            {
                trains[counter].validTrain = true;
                Serial.printf( "      validTrain:         true\n");
            }
            else
            {
                trains[counter].validTrain = false;
                Serial.printf( "      validTrain:         false\n");
            }
            
            Serial.printf( "      planned departure:  %s.%s.%s %s:%s\n", trains[counter].plannedDepartureDD, trains[counter].plannedDepartureMM, trains[counter].plannedDepartureYY, trains[counter].plannedDepartureHH, trains[counter].plannedDeparturemm);
            Serial.printf( "      actual departure:   %s.%s.%s %s:%s\n", trains[counter].actDepartureDD, trains[counter].actDepartureMM, trains[counter].actDepartureYY, trains[counter].actDepartureHH, trains[counter].actDeparturemm);
            Serial.printf( "      delay departure:    %d\n", trains[counter].delayDeparture);

            sprintf(tmpString,"%s.%s.%s %s:%s|%s:%s|%d\n", trains[counter].plannedDepartureDD, trains[counter].plannedDepartureMM, trains[counter].plannedDepartureYY, trains[counter].plannedDepartureHH, trains[counter].plannedDeparturemm, trains[counter].actDepartureHH, trains[counter].actDeparturemm, trains[counter].delayDeparture);
            strcat(BTbuffer,tmpString);
        }
    }

    display.setRotation(1);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    int16_t tbx, tby; uint16_t tbw, tbh;
    display.getTextBounds(BTbuffer, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t x = 0;// (display.width() - tbw) / 2;
    uint16_t y = 20;// (display.height() + tbh) / 2; // y is base line!
    display.setFullWindow();
    display.firstPage();
    do
    {
      display.fillScreen(GxEPD_WHITE);
      display.setCursor(x, y);
      display.print(BTbuffer);
    }
    while (display.nextPage());
}

// one time setup called from arduino framework
void setup() 
{
    int arrayCounter = 0;

    // begin serial connection for serial 
    Serial.begin(115200);

    Serial.print("\n--- trainDelay ---\n");

    display.init(115200);

    char BTbuffer[] = "--- trainDelay ---";

    display.setRotation(1);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    int16_t tbx, tby; uint16_t tbw, tbh;
    display.getTextBounds(BTbuffer, 0, 0, &tbx, &tby, &tbw, &tbh);
    Serial.printf("display width: %d height: %d\n", display.width(), display.height());
    uint16_t x = (display.width() - tbw) / 2;
    uint16_t y = (display.height() + tbh) / 2; // y is base line!
    display.setFullWindow();
    display.firstPage();
    do
    {
      display.fillScreen(GxEPD_WHITE);
      display.setCursor(x, y);
      display.print(BTbuffer);
    }
    while (display.nextPage());

    /*Serial.printf("connecting to WIFI network %s... ", WIFI_SSID);

    if(connectToWifi(WIFI_SSID, WIFI_password) == true)
    {
        Serial.print("connected\n");
    }
    else
    {
        Serial.print("timeout\n");

        return;
    }*/
}

// loop function called from arduino framework
void loop() 
{
    Serial.printf("connecting to WIFI network %s... ", WIFI_SSID);

    if(connectToWifi(WIFI_SSID, WIFI_password) == true)
    {
        Serial.print("connected\n");
    }
    else
    {
        Serial.print("timeout\n");
    }

    Serial.print("Get actual time from NTP... ");

    if(getNtpTime() == true)
    {
        /*Serial.print(day);
        Serial.print(".");
        Serial.print(month);
        Serial.print(".");
        Serial.print(year);
        Serial.print(" ");
        Serial.print(hour);
        Serial.print(":");
        Serial.print(minute);
        Serial.print(":");
        Serial.println(second);*/
        Serial.printf("%2d.%2d.%d %2d:%2d:%2d\n", day, month, year, hour, minute, second);
    }
    else
    {
        Serial.print("failed");
    }

    Serial.printf("Check if saved hour %d differs from NTP hour %d... ", actHour, (int)hour);

    if(actHour != hour)
    {
        Serial.print("does differ: save new NTP time\n");
        actHour = hour;

        Serial.print("reset plannedArrayCounter and delete previous found trains\n");
        plannedArrayCounter = 0;

        Serial.printf("Get planned timetable for departure station %s for this hour... ", departureStation);
        if(getPlannedTimetable(departureStation, hour) == true)
        {
            Serial.print("done\n");
        }
        else
        {
            Serial.print("failed\n");
        }

        Serial.print("process planned timetable for this hour... ");
        if(httpsProcessPlannedTimetable() == true)
        {
            Serial.print("done\n");
        }
        else
        {
            Serial.print("failed\n");
        }

        Serial.print("end https connection\n");
        http.end();

delay(100);

        Serial.printf("Get planned timetable for departure station %s for next hour... ", departureStation);
        if(getPlannedTimetable(departureStation, hour + 1) == true)
        {
            Serial.print("done\n");
        }
        else
        {
            Serial.print("failed\n");
        }

        Serial.print("process planned timetable for next hour... ");
        if(httpsProcessPlannedTimetable() == true)
        {
            Serial.print("done\n");
        }
        else
        {
            Serial.print("failed\n");
        }

        Serial.print("end https connection\n");
        http.end();

delay(100);

        Serial.print("get deviated timetable ... ");
        if(getDeviatedTimetable(departureStation) == true)
        {
            Serial.print("done\n");
        }
        else
        {
            Serial.print("failed\n");
        }

        Serial.print("process https data (deviated)...\n");
        if(httpsProcessDeviatedTimetable() == true)
        {
            Serial.print("done\n");
        }
        else
        {
            Serial.print("failed\n");
        }

        Serial.print("printTrains...\n");
        printTrains();

        Serial.print("end https connection\n");
        http.end();
    }
    else
    {
        Serial.print("does not differ\n");

        Serial.print("get deviated timetable ... ");
        if(getDeviatedTimetable(departureStation) == true)
        {
            Serial.print("done\n");
        }
        else
        {
            Serial.print("failed\n");
        }

        Serial.print("process https data (deviated)...\n");
        if(httpsProcessDeviatedTimetable() == true)
        {
            Serial.print("done\n");
        }
        else
        {
            Serial.print("failed\n");
        }

        Serial.print("printTrains...\n");
        printTrains();

        Serial.print("end https connection\n");
        http.end();
    }

    // idle
    Serial.print("wait for 1 minute\n");
    delay(30000);//delay(60000);
}
