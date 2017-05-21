//#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#define USE_SERIAL Serial

class Updater
{
  public:
    Updater()
    {
    };

    void GetUpdate(char *fromUrl)
    {
        USE_SERIAL.printf("try to get bin file from URL %s\r\n", fromUrl);
        t_httpUpdate_return ret = ESPhttpUpdate.update(fromUrl);
        switch (ret)
        {
        case HTTP_UPDATE_FAILED:
            USE_SERIAL.printf("HTTP_UPDATE_FAILD Error (%d): %s\r\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            USE_SERIAL.println("HTTP_UPDATE_NO_UPDATES");
            break;

        case HTTP_UPDATE_OK:
            USE_SERIAL.println("HTTP_UPDATE_OK");
            break;
        }
    }

};

extern "C" Updater *Updater_create()
{
    return new Updater();
};
extern "C" void Updater_dispose(Updater *updaterClass)
{
    delete static_cast<Updater *>(updaterClass);
};
extern "C" void Updater_do(Updater *updater, char *fromUrl)
{
    static_cast<Updater *>(updater)->GetUpdate(fromUrl);
};