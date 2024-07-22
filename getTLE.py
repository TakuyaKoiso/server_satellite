# python getTLE.py CATALOG_NUMBER SATELLITE_NAME とコマンドライン引数とともに実行する

import requests
import datetime
import MySQLdb
import sys

dt_now = datetime.datetime.now()                                # 現在時刻の取得

ses = requests.Session()                                        # セッションモードの開始
st = ses.post("https://www.space-track.org/ajaxauth/login",     # ログイン情報をPOST
              data={"identity":"koisot0928@gmail.com",
                      "password":"PazMJNfDPn5GrLf",
                      "mode":"login"})

print(dt_now.strftime("%Y-%m-%d %H:%M:%S")+" "+sys.argv[1]+" "+sys.argv[2]+" login_status : "+str(st.status_code))                    # ログインができたかどうか確認

st = ses.get("https://www.space-track.org/basicspacedata/query/class/tle/NORAD_CAT_ID/"+sys.argv[1]+"/orderby/EPOCH%20desc/limit/1/format/tle/metadata/true/emptyresult/show", timeout=(30.0, 60.0)) # クエリを送信してTLEをゲット

TLE = st.text                                                   # GETしたTLEをテキストに変換


###########################################################################################################################
# ここからTLEからの情報抽出パート

Epoch = TLE[18:32]                                                      # TLEから元期を抽出

First_deriv = float(TLE[33:43]) * 2                                     # TLEから平均運動の1次微分値を抽出

Second_deriv_sign = TLE[44]                                             # TLEから平均運動の2次微分値の符号を抽出
Second_deriv_coef = float("0."+TLE[45:50])                              # TLEから平均運動の2次微分値の仮数を抽出
Second_deriv_exp = int(TLE[50:52])                                      # TLEから平均運動の2次微分値の指数を抽出

if Second_deriv_sign == '-':                                            # 得られた符号・仮数・指数から平均運動の2次微分値を計算，符号の正負で場合分けして計算
    Second_deriv = -1 * Second_deriv_coef * 10**Second_deriv_exp
else :
    Second_deriv = Second_deriv_coef * 10**Second_deriv_exp

BSTAR_sign = TLE[53]                                                    # TLEからB*抗力項の符号を抽出
BSTAR_coef = float("0."+TLE[54:59])                                     # TLEからB*抗力項の仮数を抽出
BSTAR_exp = int(TLE[59:61])                                             # TLEからB*抗力項の指数を抽出

if BSTAR_sign == '-':                                                   # 得られた符号・仮数・指数からB*抗力項を計算，符号の正負で場合分けして計算
    BSTAR = -1 * BSTAR_coef * 10**BSTAR_exp
else :
    BSTAR = BSTAR_coef * 10**BSTAR_exp

Element = TLE[64:68]                                                    # TLEから通番を抽出
Checksum_1 = TLE[68]                                                    # TLEから1行目のチェックサムを抽出
Inclination = TLE[79:87]                                                # TLEから軌道傾斜角を抽出
Ascension = TLE[88:96]                                                  # TLEから昇交点赤経を抽出
Eccentricity = "0."+TLE[97:104]                                         # TLEから離心率を抽出
Perigee = TLE[105:113]                                                  # TLEから近地点離角を抽出
Anomaly = TLE[114:122]                                                  # TLEから平均近地点角を抽出
Motion = TLE[123:135]                                                   # TLEから平均運動を抽出
Revolution = TLE[135:139]                                               # TLEから通算周回数を抽出
Checksum_2 = TLE[139]                                                   # TLEから2行目のチェックサムを抽出

###########################################################################################################################
# ここからMySQLへ情報を入れるパート

connection = MySQLdb.connect(                                           # データベースの接続
    host='localhost',
    user='user_sat_TLE',
    passwd='9ZnSZMUEN#',
    db='sat_TLE')

cursor = connection.cursor()                                            # カーソルの生成
cursor.execute("insert into "+sys.argv[2]+" (get_datetime, raw_data, Epoch, First_derivative, Second_derivative, BSTAR, Element, Checksum1, Inclination, Ascension, Eccentricity, Perigee, Anomaly, Motion, Revolution, Checksum2) values(\'"+str(dt_now)+"\', \'"+TLE+"\', "+Epoch+", "+str(First_deriv)+", "+str(Second_deriv)+", "+str(BSTAR)+", "+Element+", "+Checksum_1+", "+Inclination+", "+Ascension+", "+Eccentricity+", "+Perigee+", "+Anomaly+", "+Motion+", "+Revolution+", "+Checksum_2+")")
                                                                        # テーブルにinsert

connection.commit()                                                     # 実行
connection.close()                                                      # 終了
