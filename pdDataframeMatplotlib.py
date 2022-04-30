#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu Apr 28 10:12:22 2022

@author: christiandoeller
"""

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates

# import csv file
csvName = 'LOG11'
data = pd.read_csv (r'/Users/christiandoeller/Dropbox/Projekte/2022_KHM_Phytotron/DataLogger/dataTesting/' + csvName + '.csv')
# file name for plot image
fileName = '_plot.png'
# variable for width of plot
figWidth = 50

print (data.head())

# make variable with sensor data name
sensorSpecs = data.columns[1]

# analyze the sensor settings - delay time and what kind of sensor
# delaytime - temp - gas - sound - light
specsList = list(sensorSpecs.split("-"))
delayTime = specsList[0]
sensorType = ''
if (specsList[1]): 
    sensorType = 'temperature'
if (specsList[2]):
    sensorType = 'airQuality'
if (specsList[3]):  
    sensorType = 'sound'    
if (specsList[4]):
    sensorType = 'light'

# make data frame with only time and sensor values
df = pd.DataFrame(data,columns=['datetime', sensorSpecs])

# rename the columns
df = df.rename (columns={sensorSpecs: sensorType})

# make pandas datetime index 
df['datetime'] = pd.to_datetime(df['datetime'])

print (df.head())

# set up Plot with specific size
fig, ax = plt.subplots(1, figsize=(figWidth,5))

# format the date structure in graph
fig.autofmt_xdate()
xfmt = mdates.DateFormatter('%d-%m-%y %H:%M:%S')
ax.xaxis.set_major_formatter(xfmt)

#turn on grid
plt.grid(True)

# rotate the date labels
plt.setp(plt.gca().xaxis.get_majorticklabels(),'rotation', 90)

# set distance of date labels every minute
plt.gca().xaxis.set_major_locator(mdates.MinuteLocator())

# make the plot with datetime and sensor data
plt.plot(df['datetime'],df[sensorType])

# set the title with delay time information
plt.title(csvName + ".csv, " + sensorType + " sensor," + " delay time " + delayTime + " milliseconds")

# save plot as image with little whitespace
plt.savefig(csvName + '.pdf', bbox_inches='tight')

# show the plot
plt.show()