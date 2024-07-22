from skyfield.api import EarthSatellite, Topos, load

# TLE データ
line1 = '1 25544U 98067A   24204.36926792  .00018812  00000-0  33458-3 0  9992'
line2 = '2 25544  51.6379 141.0388 0010232  94.1660  52.9482 15.50143306464004'

# 観測地点の緯度経度（例：東京タワー）
latitude_deg = 35.6581  # 緯度（度）
longitude_deg = 139.7414  # 経度（度）
elevation_m = 10  # 標高（メートル）

# TLE データを使って衛星オブジェクトを作成
satellite = EarthSatellite(line1, line2)

# 観測地点の情報を作成
observer = Topos(latitude_deg, longitude_deg, elevation_m)

# skyfield のロード
ts = load.timescale()

# 衛星が観測地点で可視になる時刻を計算
t_start = None
t_end = None

for time_offset in range(0, 43200, 1):  # 0から43200秒までの間隔で探索（1秒ごと）
    t = ts.now() + time_offset / 86400.0  # 1日を86400秒として計算
    topocentric = (satellite - observer).at(t)
    alt, az, distance = topocentric.altaz()
    if alt.degrees > 0:  # 衛星が地平線上にある場合
        if t_start is None:
            t_start = t
        t_end = t

if t_start is not None and t_end is not None:
    print("衛星が可視になる時刻:")
    print("開始:", t_start.utc_strftime("%Y-%m-%d %H:%M:%S"))
    print("終了:", t_end.utc_strftime("%Y-%m-%d %H:%M:%S"))
else:
    print("この時刻範囲内で衛星は見えません。")

