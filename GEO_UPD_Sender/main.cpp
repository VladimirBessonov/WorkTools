#include "pch.h"


using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices::Sensors;
using namespace Windows::Devices::Geolocation;

#define DESTINATION_PORT 54000 // for lookup UDP telegram 

int main() {


    WSADATA data;
    WORD version = MAKEWORD(2, 2);
    int wsOK = WSAStartup(version, &data);
    if (wsOK != 0) {
        std::cout << "can't start Winsock " << wsOK;
        return 0;
    }


    Geolocator geolocator = Geolocator();

    auto accessStatus{ geolocator.RequestAccessAsync() };
    /*   Geoposition pos();*/
    auto Status = accessStatus.get();
    switch (Status) {
    case (GeolocationAccessStatus::Allowed):
        // notify user: Waiting for update
        std::cout << "Access to Geo Location is allowed " << std::endl;


        break;

    case (GeolocationAccessStatus::Denied):

        std::cout << "Access to Geo Location is denied " << std::endl;
        break;

    case (GeolocationAccessStatus::Unspecified):

        std::cout << "Access to Geo Location is unspecified  " << std::endl;
        break;
    }

    std::cout << "Set IBA to listen UDP on 127.0.0.1:" << DESTINATION_PORT << std::endl;

    double lat = 0.0;
    double lon = 0.0;

    char gpsframe[16];
    memcpy(&gpsframe[0], &lat, 8);
    memcpy(&gpsframe[8], &lon, 8);

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(DESTINATION_PORT);

    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    SOCKET out = socket(AF_INET, SOCK_DGRAM, 0);

    while (true) {
        std::this_thread::sleep_for(std::chrono::microseconds(5000));
        if (Status == GeolocationAccessStatus::Allowed) {
            auto position = geolocator.GetGeopositionAsync().get();
            lat = position.Coordinate().Latitude();
            lon = position.Coordinate().Longitude();
         //   std::cout << " ALT: " << position.Coordinate().Altitude().Value() << std::endl;
                    //    int sendOk = sendto(out, s.c_str(), s.size() + 1 , 0, (sockaddr*)&server, sizeof(server));
            int sendOk = sendto(out, gpsframe, sizeof(gpsframe), 0, (sockaddr*)&server, sizeof(server));
            std::cout << "\r" << "LAT: " << *(double*)gpsframe << " LON: " << *(double*)(gpsframe + 8) << std::flush;
            if (sendOk == SOCKET_ERROR) {
                std::cout << "" << std::endl;
            }
        }
        else {
            std::cout << "\r" << "NO GPS available" << std::flush;
        }
    }

    closesocket(out);
    std::cin.get();
    return 0;
}
