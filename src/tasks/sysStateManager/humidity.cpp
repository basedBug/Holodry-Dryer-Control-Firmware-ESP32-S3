#include "humidity.h"

/*
    Compute the absolute humidity based on current temperature and
    relative humidity. Input temperature is in celsius.The output water 
    contents is in g/m^3.

    Obtained from: "Humidity at a glance" by Sensirion (2025)
*/
float absHumidity(float temp, float relHum)
{
    float absHum = 216.7 * ( (relHum/100.0) * (6.112*exp( (17.62*temp)/(243.12+temp) ))/(273.15+temp) );
    return absHum;
}