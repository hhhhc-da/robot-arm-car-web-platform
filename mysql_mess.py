# coding=utf-8
import pymysql, datetime, random

pwd = ''

arg_kwargs={
    'host':"localhost",
    'port':3308,
    'user':'root',
    'password':pwd,
    'database':"manage",
    'charset':'utf8mb4'
}
db = pymysql.connections.Connection(**arg_kwargs)

cursor = db.cursor()

cursor.execute("SELECT id, time FROM request")
records = cursor.fetchall()

now = datetime.datetime.now()

start_time = now - datetime.timedelta(hours=36)
end_time = now

for record in records:
    random_time = start_time + (end_time - start_time) * random.random()
    cursor.execute("UPDATE request SET time = %s WHERE id = %s", (random_time, record[0]))

db.commit()

cursor.close()
db.close()
