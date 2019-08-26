// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

const {google} = require('googleapis');

/*
  Variable block. Should be the only pieces needing editing
*/

// Note, this is BAD for production, don't do this. The service account
// private data should be passed into this function so it's encrypted in
// transit, and so you don't have it static as you may want to change
// permission levels, etc. For the purposes of this blog demo code, I've
// done it for ease of calling and to just show what the data looks like.
// Please please don't do this in production.
var serviceAccount = {
  "type": "service_account",
  "project_id": "dotuanson",
  "private_key_id": "667c4b940b6273df18ff1eeeeb670bbe5a3aca03",
  "private_key": "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQC+Ls9tr5gBhwtn\nx1c1T7ceNP5SA76lwJUwEr/sCZN7Z3lQ8proDTLg72aOD+1DUFm1L1x8bRrfuGN3\nTyDZMavuVq2WzHn4WjeE94VHxvjd4C7oDBwcDj9Fms5omF1peE5GFL5g/FcaFPj2\nAsXl9G7SVT0I1wLENAGdKUUa80BYRW5pQuplGk46FsnhIeN8AY+/ZL3ed5k36ek+\nPPBlTEiqvsVzoPEAEGufnKyeYtMzjZ9QbZZeWPG65fyF4sUcjD0LfUrG6//UFGya\ntZ/yMhcROfMbaJS0t+hjXvxQJtpaGQWlqo2fgJPJeK0RrY0T6X0saBwDqqPMolMR\nD6ZIFWAVAgMBAAECggEAGvJ7Bampuh2uagf+vVkMM0WC+4/4RXgiK2ucsg8xTHz+\n3Y9ij5q/KMwDFgH/tlf+PgP18NNsyWJRBai6mMvG9sBlXEagyaXO1BJWWjNBM/79\n5ndAzGDgajZjJMLiCX24aYmq6AcZOGLcW89MxyZ5RWg6e7Qrei18RQ6B8i2hYmBF\nmsE+cdipA51v0aYDqx+f6suetZD+n3AzbQECySKeV/o6FeWYl82J9KL6VjRB8a/O\nf0GWETkZY4T/fAm+hwxEIlNuB9SQu+zKj3eudFskTkNYgXYAQ6ZO+GpJ2S4qT7f0\nVB/GVTWRxG90zUStLsYyHMaSkmTdRNCDKFdGFoRWyQKBgQDeSFbbeiQzO1vtk4OA\nbnJ5JfrWjnsvdl0ptb70XY9SNmOoNayMWRMkm/KCYqFR0HSmZSrx7VuIFsicxlLe\nzfB+qkDGB5rxh8OHyRst4lMXryrUxmMA4AMBZHuh/F9K6ZJsqL3UDhXK5zRLBVAS\n9MXB6KJDbgaPjOTFJHY57JNtFwKBgQDbB/oVx6SzKjhsLiPkUTsyUkzOFQ2OMIR0\nLuICJXkoquzzAPtv6rGUzrUqXcJF84HymqXoPL5DooFPLb8i4WprN57DLrtJ8Uxb\n9lFWG0unZrM9S/cjox928BOlN7bsSwaDe4QQDO0Z4S+P9Q6CPexLa6xH0unWHmOE\nTpDBRhpPswKBgGHLD7OVxBFqHyKh0KwTa3eTVw1IODgEai5skGtwMhroZOXfX1+L\n3NkxZvsoK3MUNA302nT/iVKRO4eF1XXCwvjnLIIzXF7A7SxzNOr19LqpzT05XttG\nsuvM6nu0zbPmmQ4HGyUNYVcYazBKURVV7R9HmwPxhx9mzEtTpFQNmjrXAoGBAKm6\nf7ga8e0InZpvEs8dh3LSRcQ4mj2CKVHi9q5VHZM/CMiyDSXHx36Zw9eluiL8tMkr\nv768O3Ar4Gpb985uwUl+W1/Cuc1t2A83TAYetrxtIB4y3LtX78z58y9Hgk+PP+fp\ncggCDEXOs02px2lz1MUJFgK+VQfgmptvSksLKN5zAoGBANsaXr4vXX183vQJNx25\n41NuGg/P/hYj4CEC3HhmiHyutlZHaudUm5CYsfet37x751oywQK0jxrv2G5eBKuY\n8QJhQBCPKX+jCn9iTfuKnCxQu/hgO1zW80Lfa9uRfKHEjxyDjsTL6aCNL6ASIt8x\n2WRSIBT8PqwKA8Ea3QPkR/1q\n-----END PRIVATE KEY-----\n",
  "client_email": "bucket-20@dotuanson.iam.gserviceaccount.com",
  "client_id": "106486111496338218070",
  "auth_uri": "https://accounts.google.com/o/oauth2/auth",
  "token_uri": "https://oauth2.googleapis.com/token",
  "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
  "client_x509_cert_url": "https://www.googleapis.com/robot/v1/metadata/x509/bucket-20%40dotuanson.iam.gserviceaccount.com"
};

// You shouldn't need to change this variable, it's here for illustration
// purposes. An advantage of calling a Cloud Function within the same project
// is that there are certain environment varibales you get for free like this
// one
var projectId = process.env.GCP_PROJECT;

// change the following to match your project's values
// you could also easily modify the code to receieve these as variables
// in the GET call since I'm relying on that for the config/command switch
// as well as the actual message being sent
var registryId = 'my-registry';
var gcpLocation = 'asia-east1';
var deviceId = 'MQTT';

// a couple default values just for the sake of having something there
var msg = "clear"; // by default will reset the LED matrix
var which = "config"; // by default will send a config message

/*
  END VARIABLE BLOCK
*/

/*
  Responds to a HTTP request and updates the specified
  IoT device with either a config or a command depending on
  what is specified in the GET parameter (defaults to config).
 
  @param {Object} req Cloud Function request context.
  @param {Object} res Cloud Function response context.
*/
exports.updateDevice = (req, res) => {
  // variable passed into the Cloud function (as GET or POST)
  const reqMsg = req.query.message;
  // if we didn't pass one in, then just take our default
  const finalMsg = reqMsg ? reqMsg : msg;
  const msgData = Buffer.from(finalMsg).toString('base64');
  deviceId =  req.query.deviceID ? req.query.deviceID : 'MQTT';
  
  // This chunk is what authenticates the function with the API so you can
  // call the IoT Core APIs
  const jwtAccess = new google.auth.JWT();
  jwtAccess.fromJSON(serviceAccount);
  // Note that if you require additional scopes, they should be specified as a
  // string, separated by spaces
  jwtAccess.scopes = 'https://www.googleapis.com/auth/cloud-platform';
  // Set the default authenticatio nto the above JWT access
  google.options({ auth: jwtAccess });

  // the full path to our device in GCP
  var devicePath = `projects/${projectId}/locations/${gcpLocation}/registries/${registryId}/devices/${deviceId}`;

  // figure out if they're trying to send config vs command
  // note the values of "which" that can be sent via the URL is "config" or "command"
  const reqTypeMsg = req.query.which;
  const finalWhich = reqTypeMsg ? reqTypeMsg : which;

  // And here we have the actual call to the cloudiot REST API for updating a
  // configuration on the device
  var client = google.cloudiot('v1');
  if (finalWhich == "config") {
    // This is the blob send to the IoT Core Admin API
    const configRequest = {
      name: devicePath,
      versionToUpdate: '0',
      binaryData: msgData
    };
    client.projects.locations.registries.devices.modifyCloudToDeviceConfig(configRequest,
      (err, data) => {
        if (err) {
          console.log('Message: ', err);
          console.log('Could not update config:', deviceId);
        } else {
          console.log('Success :', data);
        }
      }
    );
  }
  else if (finalWhich == "command") {
    const commandRequest = {
      name: devicePath,
      binaryData: msgData
    };
    client.projects.locations.registries.devices.sendCommandToDevice(commandRequest,
      (err, data) => {
        if (err) {
          console.log('Message: ', err);
          console.log('Could not update command:', deviceId);
        } else {
          console.log('Success :', data);
        }
      }
    );
  }

  res.status(200).end();
};