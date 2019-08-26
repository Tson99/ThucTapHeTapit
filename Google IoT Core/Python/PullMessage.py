import os
import json
import mysql.connector
import time
from google.cloud import pubsub_v1
os.environ["GOOGLE_APPLICATION_CREDENTIALS"]="E:\ThucTapHeTapit\Google IoT Core\Python\my_service_account.json"

'''mydb = mysql.connector.connect(
	  host="localhost",
	  user="root",
	  passwd="",
	  database="cloudpubsub"
	)'''


project_id = "dotuanson"
topic_name = "my-device-events"
subscription_name = "event-subscription"
subscriber = pubsub_v1.SubscriberClient()
subscription_path = subscriber.subscription_path(project_id, subscription_name)

'''def PushToDatabase(Data,Attributes):
	mycursor = mydb.cursor()
	sql = "INSERT INTO messages VALUES (%s, %s, %s, %s, %s, %s, %s)"
	val = (Data,Attributes["deviceId"],Attributes["deviceNumId"],Attributes["deviceRegistryId"],Attributes["deviceRegistryLocation"],Attributes["projectId"],Attributes["subFolder"])
	mycursor.execute(sql, val)
	mydb.commit()
	print(mycursor.rowcount, "record inserted.")'''

def callback(message):
    print('Received message in topic %s: {}'.format(message)%(topic_name))
    message.ack()
'''  
    #Lấy messages từ cloud PubSub, có 2 phần là data và attributes
    mess = format(message)[8:]
    mess = mess.replace("'","\"")
    mess = mess[:10] + mess[11:]

    #Xử lý data(string)
    startData = mess.find('"')
    endData = mess.find('"',startData + 1)
    Data = mess[startData:endData+1]  
    #print(Data)

    #Xử lý attributes(dict)
    startAtt = mess.find('{',1)
    endAtt = mess.find('}')
    text = mess[startAtt:endAtt+1]
    #print(text)
    Attributes = json.loads(text)

    #Đẩy dữ liệu vào SQL	
    PushToDatabase(Data,Attributes)
 '''

subscriber.subscribe(subscription_path, callback=callback)

# The subscriber is non-blocking. We must keep the main thread from
# exiting to allow it to process messages asynchronously in the background.
print('Listening for messages on {}'.format(subscription_path))
while True:
    time.sleep(60)
