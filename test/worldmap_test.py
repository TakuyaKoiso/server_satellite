import cartopy.crs as ccrs
import matplotlib.pyplot as plt
import MySQLdb
import subprocess

fig = plt.figure()
ax = fig.add_subplot(1,1,1, projection=ccrs.PlateCarree())
ax.gridlines(crs=ccrs.PlateCarree(),draw_labels=True)

conn = MySQLdb.connect(user='user_sat_TLE', password='9ZnSZMUEN#', host='localhost', database='sat_TLE')
cur = conn.cursor()
cur.execute("select Epoch,First_derivative,Inclination,Ascension,Eccentricity,Perigee,Anomaly,Motion from ISS_ZARYA order by Epoch desc limit 1")
TLE = cur.fetchall()

Epoch = TLE[0][0]
First_derivative = TLE[0][1]
Inclination = TLE[0][2]
Ascension = TLE[0][3]
Eccentricity = TLE[0][4]
Perigee = TLE[0][5]
Anomaly = TLE[0][6]
Motion = TLE[0][7]

cm = plt.cm.get_cmap('winter')

hour = -1.0
longitude_array = [] 
latitude_array = []
height_array = []
hour_array = []

while hour<24:
    result = subprocess.run([f"/home/takuya/satellite/test/location_from_tle.exe", f"{Epoch}", f"{First_derivative}", f"{Inclination}", f"{Ascension}", f"{Eccentricity}", f"{Perigee}", f"{Anomaly}", f"{Motion}", f"{hour}"], capture_output=True, text=True)
#result = subprocess.run(f"/home/takuya/satellite/location_from_tle.exe")
    position = result.stdout.split()
    longitude_array.append(float(position[7]))
    latitude_array.append(float(position[6]))
    height_array.append(float(position[8]))
    hour_array.append(hour)
    hour = hour + 0.005
    if hour > -0.00000001 and hour < 0.000000001:
        now_position = [float(position[7]), float(position[6])]

plt.scatter(longitude_array, latitude_array, c=hour_array, s=0.2, transform=ccrs.PlateCarree(), cmap=cm)
plt.colorbar()
ax.scatter(now_position[0], now_position[1], alpha=1 ,color='y', edgecolor='m', s=100, marker="*", transform=ccrs.PlateCarree())
ax.coastlines()
ax.stock_img()


fig.savefig("test.jpg", dpi=150)
