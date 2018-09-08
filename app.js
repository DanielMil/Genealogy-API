'use strict'

// C library API
const ffi = require('ffi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

const mysql = require('mysql');

app.use(fileUpload());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

const portNum = process.argv[2];

app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

app.get('/style.css',function(req,res){
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

app.post('/upload', function(req, res) {
  
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
 
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
});

app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
}); 

app.get('/images/background.png',function(req,res){
  res.sendFile(path.join(__dirname+'/public/images/background.png'));
});

app.get('/fonts/Lato-Hairline.ttf',function(req,res){
  res.sendFile(path.join(__dirname+'/public/fonts/Lato-Hairline.ttf'));
});

let sharedLib = ffi.Library('./sharedLib', {
  'createGEDCOMJSON': [ 'string', ['string'] ], 
  'createGEDCOMIndividualJSON' : [ 'string', ['string'] ],
  'createNewGEDCOM' : [ 'string', ['string', 'string'] ], 
  'addIndividualFrontEnd' : [ 'string', ['string', 'string'] ],
  'descendantsInterface' : [ 'string', ['string', 'string', 'string', 'int'] ],
  'ancestorsInterface' : [ 'string', ['string', 'string', 'string', 'int'] ] 
});

app.get('/filenames', function(req , res){
  
  let fileNames = [];
  const dir = "uploads/";
  let objectsArray = []; 

  fs.readdir(dir, (err, files) => {
    files.forEach(file => { 
      let objects = "";
      let path = "uploads/"; 
      path += file;
      objects = sharedLib.createGEDCOMJSON(path);

      if (objects != "") {
        objectsArray.push(objects); 
      } 

    }) 

    res.send(objectsArray); 

  })
});

app.get('/individualData', function(req, res) {
  
  let fileNamePath = req.query.fileName; 
  let object = sharedLib.createGEDCOMIndividualJSON(fileNamePath);  
  let json = JSON.parse(object);

  res.send(json); 

});

app.get('/createGEDCOM', function(req, res){
 
  let JSONIndividual = req.query.JSON;
  let toJSON = JSON.parse(JSONIndividual); 
  let filepath = toJSON.filename;
  let jsonInC = '{"encoding":"ASCII","source":"DanielMilGEDCOMAPI","gedcVersion":"1.0","subName":"' + toJSON.subname + '","subAddress":"' + toJSON.subaddress + '"}';
  
  let result = sharedLib.createNewGEDCOM(filepath, jsonInC); 
 
  res.send(result); 

});  

app.get('/addIndividual', function(req, res) {
 
  let JSONIndividual = req.query.JSON;
  let toJSON = JSON.parse(JSONIndividual); 
  let filepath = toJSON.filename;
  let jsonInC = '{"givenName":"' + toJSON.givenName + '","surname":"' + toJSON.surname + '"}'; 

  let result = sharedLib.addIndividualFrontEnd(filepath, jsonInC); 

  res.send(result); 
  
});

app.get('/getDescendants', function(req, res) {
  
  let JSONstring = req.query.JSONstring;
  let toJSON = JSON.parse(JSONstring); 
  let filepath = toJSON.filename;
  let givenName = toJSON.givenName;
  let surname = toJSON.surname;
  let maxGen = toJSON.maxGen;

  if (parseInt(maxGen) <= 0)
    maxGen = 0;

  let result = sharedLib.descendantsInterface(filepath, givenName, surname, maxGen); 

  res.send(result); 

});

app.get('/getAncestors', function(req, res) {
  
  let JSONstring = req.query.JSONstring;
  let toJSON = JSON.parse(JSONstring); 
  let filepath = toJSON.filename;
  let givenName = toJSON.givenName;
  let surname = toJSON.surname;
  let maxGen = toJSON.maxGen;

  if (parseInt(maxGen) <= 0)
    maxGen = 0;

  let result = sharedLib.ancestorsInterface(filepath, givenName, surname, maxGen); 

  res.send(result); 

});

let connection = null; 
app.get('/login', function(req, res) {
  
  let input = req.query.JSON; 
  let inputJSON = JSON.parse(input);

  connection = mysql.createConnection({
    host     : 'dursley.socs.uoguelph.ca',
    user     :  inputJSON.username,
    password :  inputJSON.password,
    database :  inputJSON.db
  });

  let valid = true;

  if (connection == null) {
    res.send("Invalid username or password.");
    return; 
  }

  connection.query("create table FILE ( file_id int not null auto_increment primary key, file_Name VARCHAR(60) NOT NULL, source VARCHAR(250) NOT NULL, version VARCHAR(10) NOT NULL, encoding VARCHAR(10) NOT NULL, sub_name VARCHAR(62) NOT NULL, sub_addr VARCHAR(256), num_individials INT DEFAULT 0, num_families INT DEFAULT 0)", function (err, rows, fields) {
      if (err) {
        if (err.toString().includes("ER_ACCESS_DENIED_ERROR")) {
          valid = false;
        }
        console.log("Something went wrong creating tables: " + err);
      }
  });

  connection.query("create table INDIVIDUAL ( ind_id int not null auto_increment primary key, surname VARCHAR(256) NOT NULL, given_name VARCHAR(256) NOT NULL, sex VARCHAR(1), fam_size INT NOT NULL DEFAULT 0, source_file INT, FOREIGN KEY(source_file) REFERENCES FILE(file_id) ON DELETE CASCADE)", function (err, rows, fields) {
      if (err) {
        if (err.toString().includes("ER_ACCESS_DENIED_ERROR")) {
          valid = false; 
        }
        console.log("Something went wrong creating tables: " + err);
      }
  });

  setTimeout(function() {
    if (!valid) {
      res.send("Error");
    } else {
      res.send("Successfully logged into database."); 
    }
  }, 200);

});

app.get('/storeFiles', function(req, res) {
  
  let files = req.query.JSONFiles;
  let arr = JSON.parse(files);

  connection.query("DELETE FROM FILE", function(err, rows, fields) {
    if (err) {
      console.log("Error deleting rows from FILE"); 
    } else { 
      connection.query("ALTER TABLE FILE auto_increment = 1", function(err, rows, fields) {
          if (err) {
            console.log("Error reseting primary key value.") 
          } else {
            for (let rec of arr) {
              let filename = rec.File.substring(8, rec.File.length); 
              connection.query("INSERT INTO FILE (file_Name, source, version, encoding, sub_name, sub_addr, num_individials, num_families) VALUES ('" + filename + "','" + rec.Source + "','" + rec.Version + "','" + rec.Encoding + "','" + rec.SubName + "','" + rec.SubAdd + "','" + rec.NumInds + "','" + rec.NumFams + "')", function (err, rows, fields) {
                if (err) console.log("Something went wrong. "+ err);
              });
            }

            let returned = [];
            returned.push("Successfully loaded database.");
            res.send(returned);
          }
      });
    }
  });

});

app.get('/storeFilesIndividual', function(req, res) {
  
  let individuals = req.query.JSONIndividuals;
  let arr = JSON.parse(individuals);

  connection.query("DELETE FROM INDIVIDUAL", function(err, rows, fields) {
    if (err) {
      console.log("Error deleting rows from INDIVIDUAL"); 
    } else { 
      connection.query("ALTER TABLE INDIVIDUAL auto_increment = 1", function(err, rows, fields) {
        for (let rec of arr) {
          connection.query("select file_id from FILE where file_Name = '" + rec.file + "'", function (err, rows, fields) {  
            if (err) console.log("Something went wrong: " + err); 
            connection.query("INSERT INTO INDIVIDUAL (surname, given_name, sex, fam_size, source_file) VALUES ('" + rec.surname + "','" + rec.givenName + "','" + rec.sex + "','" + rec.familySize + "','" + rows[0].file_id + "')", function (err, rows, fields) {
              if (err) console.log("Something went wrong. "+ err);
            });
          });
        }
      });
    }
  });
});

app.get('/deleteRows', function(req, res){

  connection.query("DELETE FROM INDIVIDUAL", function(err, rows, fields) {
    if (err) {
      console.log("Error deleting rows from INDIVIDUAL"); 
    } else { 
      connection.query("ALTER TABLE INDIVIDUAL auto_increment = 1", function(err, rows, fields) {
        if (err) console.log("Something went wrong: " + err);
      });
    }
  });  

  connection.query("DELETE FROM FILE", function(err, rows, fields) {
    if (err) {
      console.log("Error deleting rows from FILE"); 
    } else { 
      connection.query("ALTER TABLE FILE auto_increment = 1", function(err, rows, fields) {
        if (err) console.log("Something went wrong: " + err);
      });
    }
  });

});

app.get('/countRows', function(req,res) {

  let files = 0;
  let individuals = 0;

  connection.query("SELECT * FROM INDIVIDUAL", function(err,rows,fields) {
    individuals = rows.length; 
  });

  connection.query("SELECT * FROM FILE", function(err,rows,fields) {
    files = rows.length; 
  });

  setTimeout(function() {
    res.send("Database has " + files + " files and " + individuals + " individuals.");
  }, 300);

});

app.get('/sortLastName', function(req,res) {

  let arr = [];

  connection.query("SELECT * FROM INDIVIDUAL ORDER BY surname ASC, given_name ASC", function(err,rows,fields) {
    if (err) console.log(err); 
    arr = rows;  
  });

  setTimeout(function() {
    res.send(arr);
  }, 300);

});

app.get('/customQuery', function(req, res) {

  let queryString = 'SELECT ';
  let arr = [];
  queryString += req.query.customQueryString; 

  connection.query(queryString, function(err, rows, fields) {
    arr = rows;
  });

  setTimeout(function() {
    res.send(arr);
  }, 300);

});

app.get('/sortIndividualsFile', function(req,res) {

  let filename = req.query.filename;
  let queryString = "SELECT * FROM INDIVIDUAL WHERE source_file = (SELECT file_id FROM FILE WHERE file_Name = '" + filename + "')";
  let arr = [];

  connection.query(queryString, function (err,rows, fields) {
    if (err) console.log(err);
    arr = rows;
  });

  setTimeout(function() {
    console.log(arr);
    res.send(arr);
  }, 300);

});

app.get('/sortGender', function(req, res) {

  let arr = [];

  connection.query('SELECT * FROM INDIVIDUAL ORDER BY sex ASC, given_name ASC', function(err,rows,fields) {
    arr = rows; 
  });

  setTimeout(function() {
    console.log(arr);
    res.send(arr);
  }, 300);

});

app.get('/sortGiven', function(req, res) {

  let arr = [];
  let givenName = req.query.givenName;
  let queryString = 'SELECT * FROM INDIVIDUAL WHERE given_name = "' + givenName + '"';

  connection.query(queryString, function(err,rows,fields) {
    arr = rows; 
  });

  setTimeout(function() {
    console.log(arr);
    res.send(arr);
  }, 300);

});

app.get('/sortSurname', function(req, res) {

  let arr = [];
  let surname = req.query.surname;
  let queryString = 'SELECT * FROM INDIVIDUAL WHERE surname = "' + surname + '"';

  connection.query(queryString, function(err,rows,fields) {
    arr = rows; 
  });

  setTimeout(function() {
    console.log(arr);
    res.send(arr);
  }, 300);

});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
