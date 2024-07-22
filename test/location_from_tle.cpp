/*****************************************

TLEデータから衛星の緯度経度高度を計算するプログラム
hoge.exe EPOCH FIRST_DERIVATIVE INCLINATION ASCENSION ECCENTRICITY PERIGEE ANOMALY MOTION TIME_DIFF とコマンドライン引数を入れて実行する
計算精度は1次精度なので，そこまで精度は高くない．

******************************************/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cmath>
#include <ctime>

using namespace std;

#define mu 2.975537e15                                  // 地心重力定数 km^3/day^2
#define PI 3.141592653589793                            // 円周率
#define EARTH_RADI 6378.137                             // 地球半径 km

double kepler(double e, double M, double E);            // ケプラー方程式
double kepler_dif(double e, double E);                  // ケプラー方程式の微分系
double Newton_Raphson(double ini, double e, double M);  //　ケプラー方程式を解くニュートンラプソン

int main(int argc, char* argv[])                        // コマンドライン引数を定義
{

    // TLE data
    double Epoch            = stod(argv[1]);            // 元期
    double First_derivative = stod(argv[2]);            // 平均運動の1次微分値
    double Inclination      = stod(argv[3]);            // 軌道傾斜角
    double Ascension        = stod(argv[4]);            // 昇交点赤経
    double Eccentricity     = stod(argv[5]);            // 離心率
    double Perigee          = stod(argv[6]);            // 近地点離角
    double Anomaly          = stod(argv[7]);            // 平均近点角
    double Motion           = stod(argv[8]);            // 平均運動
    double TimeDiff	    = stod(argv[9]);
    
    double  Epoch_Japan;
    int     Epoch_Japan_year;                           // 元期の年(JST)
    int     Epoch_Japan_month;                          // 元期の月(JST)
    int     Epoch_Japan_day;                            // 元期の日(JST)
    double  Epoch_Japan_time;                           // 元期の時刻（小数点表記，JST）
    int     Epoch_Japan_hour;                           // 元期の時間(JST)
    int     Epoch_Japan_minute;                         // 元期の分(JST)
    int     Epoch_Japan_second;                         // 元期の秒(JST)

    int Epoch_year;                                     // 元期の年(UTC)
    double  Epoch_day;                                  // 元期の日(1月1日からの経過日，UTC)

    double inclination_rad;
    double ascension_now_deg;
    double ascension_now_rad;
    double eccentric_anomaly_rad;
    double eccentric_anomaly_deg;
    double perigee_now_deg;
    double perigee_now_rad;
    double anomaly_now;
    double motion_now;
    double coordinate_U;
    double coordinate_V;
    double coordinate_x;
    double coordinate_y;
    double coordinate_z;
    double coordinate_lX;
    double coordinate_lY;
    double coordinate_lZ;
    double JD;
    int JD_year;
    int JD_mon;
    double TJD;
    double sidereal;
    double latitude;
    double longitude;
    double height;

    double long_radius;

    double now_day = 0;
    float month_day[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    /*****************************************************************************************
     UTC表記の元期をJST表記に変換
    ******************************************************************************************/


    Epoch_Japan = Epoch + 9.0 / 24.0;
    Epoch_Japan_year = floor(Epoch_Japan/1000 + 2000);
    Epoch_Japan_day = floor(Epoch_Japan - floor(Epoch_Japan/1000)*1000);
    Epoch_Japan_time = Epoch_Japan - (double)floor(Epoch_Japan/1000)*1000 - (double)Epoch_Japan_day;

    if (Epoch_Japan_year % 400 == 0 | Epoch_Japan_year % 4 == 0 && Epoch_Japan_year % 100 != 0) {
        if (Epoch_Japan_day > 366) {
            Epoch_Japan_year += 1;
            Epoch_Japan_day -= 366;
        }
    } else {
        if (Epoch_Japan_day > 365) {
            Epoch_Japan_year += 1;
            Epoch_Japan_day -= 365;
        }
    }

    Epoch_Japan_month = 0;

    while (Epoch_Japan_day - month_day[Epoch_Japan_month] > 0) {
  
        if(Epoch_Japan_month == 1 && (Epoch_Japan_year % 400 == 0 | Epoch_Japan_year % 4 == 0 && Epoch_Japan_year % 100 != 0)) {
            Epoch_Japan_day -= 29;
        } else {
            Epoch_Japan_day -= month_day[Epoch_Japan_month];
        }
        
        Epoch_Japan_month++;
    }

    Epoch_Japan_day = floor(Epoch_Japan_day);

    if (Epoch_Japan_month == 1 && (Epoch_Japan_year % 400 == 0 | Epoch_Japan_year % 4 == 0 && Epoch_Japan_year % 100 != 0)) {
        if (Epoch_Japan_day > 30) {
            Epoch_Japan_month += 1;
            Epoch_Japan_day -= 29;
        }
    } else {
        if (Epoch_Japan_day > month_day[Epoch_Japan_month]) {
            Epoch_Japan_day -= month_day[Epoch_Japan_month];
            Epoch_Japan_month += 1;
        }
    }


    Epoch_Japan_month += 1;

    Epoch_Japan_hour = floor(Epoch_Japan_time * 24);
    Epoch_Japan_time -= (double)Epoch_Japan_hour/24;
    Epoch_Japan_minute = floor(Epoch_Japan_time * 24 * 60);
    Epoch_Japan_time -= (double)Epoch_Japan_minute / (24 * 60);
    Epoch_Japan_second = floor(Epoch_Japan_time * 24 * 60 * 60);

    std::cout << Epoch_Japan_year << '\n' << Epoch_Japan_month << '\n' << Epoch_Japan_day << '\n' << Epoch_Japan_hour << '\n' << Epoch_Japan_minute << '\n' << Epoch_Japan_second << '\n';

/*****************************************************************************************
 現在時刻から，元期と同じ表記である日付の小数点表記に変換
******************************************************************************************/

    time_t current = time(NULL);
    struct tm* UT = gmtime(&current);

    for (int i = 0; i < UT->tm_mon; i++){
        now_day += (double)month_day[i];
    }

    now_day += (double)UT->tm_mday + ((double)UT->tm_hour + TimeDiff) /24 + (double)UT->tm_min / 24 / 60 + (double)UT->tm_sec / 24 / 60 / 60; 

    if ((UT->tm_year +1900)%4 == 0 && (UT->tm_year +1900)%100 != 0 || (UT->tm_year +1900)%400 == 0) {
        now_day += 1;
    }

/*****************************************************************************************
  平均運動の一次微分値と元期での平均運動の値から，現在の平均運動を求める
******************************************************************************************/

    Epoch_year = floor(Epoch/1000 + 2000);
    Epoch_day = Epoch - floor(Epoch/1000.0)*1000.0;

    if (now_day - Epoch_day < 0){
        if(Epoch_year % 400 == 0 | Epoch_year % 4 == 0 && Epoch_year % 100 != 0) {
            motion_now = Motion + First_derivative * (now_day + (367.0 - Epoch_day));
        } else {
            motion_now = Motion + First_derivative * (now_day + (366.0 - Epoch_day));
        }
    } else {
        motion_now = Motion + First_derivative * (now_day - Epoch_day);
    }

/*****************************************************************************************
  ケプラーの法則から，軌道長半径を求める
******************************************************************************************/
    
    long_radius = cbrt(mu / (4 * pow(PI, 2) * pow(motion_now, 2)));

/*****************************************************************************************
  ケプラー方程式から現在の平均近点角を求める
******************************************************************************************/

    anomaly_now = Anomaly/360.0 + Motion*(now_day - Epoch_day) + First_derivative/2.0*(now_day - Epoch_day)*(now_day - Epoch_day);
    anomaly_now = (anomaly_now - floor(anomaly_now)) * 360.0;     // rev -> deg conversion

    eccentric_anomaly_rad = Newton_Raphson(100.0, Eccentricity, anomaly_now);
    eccentric_anomaly_deg = eccentric_anomaly_rad / (2 * PI) * 360;

/*****************************************************************************************
  人工衛星の軌道面上の現在の座標を求める
******************************************************************************************/

    coordinate_U = long_radius * cos(eccentric_anomaly_rad) - long_radius * Eccentricity;
    coordinate_V = long_radius * sqrt(1 - Eccentricity * Eccentricity) * sin(eccentric_anomaly_rad);

/*****************************************************************************************
  現在の近地点離角と昇交点赤経を計算する
******************************************************************************************/

    inclination_rad = Inclination/360*2*PI;

    perigee_now_deg = Perigee + (180.0 * 0.174 * (2.0-2.5*sin(inclination_rad)*sin(inclination_rad)))/(PI*pow(long_radius/EARTH_RADI, 3.5)) * (now_day - Epoch_day);
    ascension_now_deg = Ascension - (180.0 * 0.174 * cos(inclination_rad))/(PI * pow(long_radius/EARTH_RADI, 3.5)) * (now_day - Epoch_day);

    perigee_now_rad = perigee_now_deg/360*2*PI;
    ascension_now_rad = ascension_now_deg/360*2*PI;

/*****************************************************************************************
  人工衛星の軌道面上の現在の座標を地球中心の3次元座標に変換する
******************************************************************************************/

    coordinate_x = (cos(ascension_now_rad) * cos(perigee_now_rad) - sin(ascension_now_rad) * cos(inclination_rad) * sin(perigee_now_rad))*coordinate_U + (-cos(ascension_now_rad) * sin(perigee_now_rad) - sin(ascension_now_rad) * cos(inclination_rad) * cos(perigee_now_rad))*coordinate_V;
    coordinate_y = (sin(ascension_now_rad) * cos(perigee_now_rad) + cos(ascension_now_rad) * cos(inclination_rad) * sin(perigee_now_rad))*coordinate_U + (-sin(ascension_now_rad) * sin(perigee_now_rad) + cos(ascension_now_rad) * cos(inclination_rad) * cos(perigee_now_rad))*coordinate_V;
    coordinate_z = (sin(inclination_rad) * sin(perigee_now_rad))*coordinate_U + (sin(inclination_rad) * cos(perigee_now_rad))*coordinate_V;

/*****************************************************************************************
  観測時刻（このプログラムでは計算実行時）におけるグリニッジ子午線の赤経計算
******************************************************************************************/

    JD_year = (UT->tm_year) + 1900;
    JD_mon = (UT->tm_mon) + 1;

    if (JD_mon == 1) {
        JD_year--;
        JD_mon = 13;
    } else if (JD_mon == 2) {
        JD_year--;
        JD_mon = 14;
    }

    JD = floor(365.25*JD_year) + floor(JD_year/400) - floor(JD_year/100) + floor(30.59*(JD_mon-2)) + UT->tm_mday + 1721088.5 + ((UT->tm_hour)+TimeDiff)/24.0 + (UT->tm_min)/1440.0 + (UT->tm_sec)/86400.0;
    TJD = JD - 2440000.5;
    sidereal = 0.671262 + 1.0027379094 * TJD;
    sidereal = sidereal - floor(sidereal);

/*****************************************************************************************
  春分点がx軸だったものを，グリニッジ子午線の方向をx軸に変換
******************************************************************************************/

    coordinate_lX = cos(-2*PI*sidereal)*coordinate_x - sin(-2*PI*sidereal)*coordinate_y;
    coordinate_lY = sin(-2*PI*sidereal)*coordinate_x + cos(-2*PI*sidereal)*coordinate_y;
    coordinate_lZ = coordinate_z;

/*****************************************************************************************
  3次元直交座標を緯度経度に変換
******************************************************************************************/

    latitude = asin(coordinate_lZ/sqrt(coordinate_lX*coordinate_lX + coordinate_lY*coordinate_lY + coordinate_lZ*coordinate_lZ)) * 360/(2*PI);
    longitude = atan2(coordinate_lY, coordinate_lX) * 360/(2*PI);
    height = sqrt(coordinate_lX*coordinate_lX + coordinate_lY * coordinate_lY + coordinate_lZ * coordinate_lZ) - EARTH_RADI;

    std::cout << std::fixed;
    std::cout << std::setprecision(3) << latitude << '\n' << longitude << '\n' << height << '\n';

    return 0;
}

double kepler(double e, double M, double E)
{
    double M_rad;
    M_rad = M/360.0*2.0*PI;

    return E - e*sin(E) - M_rad;
}

double kepler_dif(double e, double E)
{
    return 1.0 - e*cos(E);
}

double Newton_Raphson(double ini, double e, double M)
{
    double E;

    E = ini;

    while (fabs(kepler(e, M, E)) > 0.00000000001) {
        E = E - kepler(e, M, E) / kepler_dif(e, E);
    }

    return E;
}
