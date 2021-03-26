#include "pch.h"
#include <Windows.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices::Sensors;
using namespace Windows::Devices::Geolocation;

#define DESTINATION_PORT 54000 // for lookup UDP telegram 

void gotoxy(int column, int line)
{
    COORD coord;
    coord.X = column;
    coord.Y = line;
    SetConsoleCursorPosition(
        GetStdHandle(STD_OUTPUT_HANDLE),
        coord
    );
}

int wherex()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD                      result;
    if (!GetConsoleScreenBufferInfo(
        GetStdHandle(STD_OUTPUT_HANDLE),
        &csbi
    ))
        return -1;
    return result.X;
}

int wherey()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD                      result;
    if (!GetConsoleScreenBufferInfo(
        GetStdHandle(STD_OUTPUT_HANDLE),
        &csbi
    ))
        return -1;
    return result.Y;
}

int main() {
    char emptyline[200] = "                                                                       ";
    std::cout << "Start... Checking if Geolocation is enabled on this laptop." << std::endl;
    WSADATA data;
    WORD version = MAKEWORD(2, 2);
    int wsOK = WSAStartup(version, &data);
    if (wsOK != 0) {
        std::cout << "can't start Winsock " << wsOK;
        return 0;
    }




    double lat = 0.0;
    double lon = 0.0;
    double Mocklat = 0.0;
    double Mocklon = 0.0;
    char gpsframe[16];
   

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(DESTINATION_PORT);

    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    SOCKET out = socket(AF_INET, SOCK_DGRAM, 0);

    Geolocator geolocator = Geolocator();
  
    while (true) {

        auto accessStatus{ geolocator.RequestAccessAsync() };
        /*   Geoposition pos();*/
        auto Status = accessStatus.get();
        switch (Status) {
        case (GeolocationAccessStatus::Allowed):
            // notify user: Waiting for update
            printf("%c[2K", 27);
            std::cout << "\r" << "Access to Geo Location is allowed\n" << std::endl;
            std::cout <<  "Set IBA to listen UDP on 127.0.0.1:" << DESTINATION_PORT << "\n" << std::endl;

            break;

        case (GeolocationAccessStatus::Denied):
            printf("%c[2K", 27);
            std::cout << "\r" << "Access to Geo Location is denied\n " << std::endl;
            std::cout <<  "Set IBA to listen UDP on 127.0.0.1:" << DESTINATION_PORT << "\n" << std::endl;
            break;

        case (GeolocationAccessStatus::Unspecified):
            printf("%c[2K", 27);
            std::cout << "Access to Geo Location is unspecified\n" << std::flush;
            break;
        }

        
        std::this_thread::sleep_for(std::chrono::microseconds(5000));
        if (Status == GeolocationAccessStatus::Allowed) {

            try { /* */ 
                auto position = geolocator.GetGeopositionAsync().get();
                auto Setelite = position.Coordinate().SatelliteData();
               // double PDOP =  Setelite.PositionDilutionOfPrecision().Value();
                lat = position.Coordinate().Latitude();
                lon = position.Coordinate().Longitude();
               
            //    GeocoordinateSatelliteData setelite();
                

                memcpy(&gpsframe[0], &lat, 8);
                memcpy(&gpsframe[8], &lon, 8);

                int sendOk = sendto(out, gpsframe, sizeof(gpsframe), 0, (sockaddr*)&server, sizeof(server));
                std::cout << "LAT: " << lat << " LON: " << lon <<  std::endl;
                if (sendOk == SOCKET_ERROR) {
                    std::cout << "" << std::endl;
                }
                gotoxy(0, 1);
            }
            catch (const std::exception& e) { /* */ 
                std::cout << "Exception: " << e.what() << std::endl;
            }


        }
        else {
            memcpy(&gpsframe[0], &Mocklat, 8);
            memcpy(&gpsframe[8], &Mocklon, 8);
            int sendOk = sendto(out, gpsframe, sizeof(gpsframe), 0, (sockaddr*)&server, sizeof(server));
            std::cout <<  "Mock Geolocation as NO GPS available: " << "LAT: " << *(double*)gpsframe << " LON: " << *(double*)(gpsframe + 8) << std::flush;
            if (sendOk == SOCKET_ERROR) {
                std::cout << "" << std::endl;
            }
            gotoxy(0, 1);
        }
    }

    closesocket(out);
    std::cin.get();
    return 0;
}
