#!/bin/bash

# Get concurrent users and number of requests as parameters
CONCURRENT_USERS=$1
NUMBER_OF_REQUESTS=$2

# Set the path to the URL list file
URL_LIST_FILE="/workspaces/webserv/siege/urls.txt"

# Run the Siege command with the specified parameters
siege -c $CONCURRENT_USERS -r $NUMBER_OF_REQUESTS -f $URL_LIST_FILE -b -i
