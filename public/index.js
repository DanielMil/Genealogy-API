$(document).ready(function() {

    let filenames = [];
    let fileTable = [];
    let individualTable = [];

    reload(); 

    let index = 1;

    /*MODAL CODE*/
    var modal = document.getElementById('myModal');
    var span = document.getElementsByClassName("close")[0];
    modal.style.display = "block";
    /*END MODAL CODE*/

    /*HELP MODAL CODE*/
    var modalHelp = document.getElementById('help-panel');
    var btn = document.getElementById("help-button");
    var span = document.getElementsByClassName("close-help")[0];
    btn.onclick = function() {
        modalHelp.style.display = "block";
    }
    span.onclick = function() {
        modalHelp.style.display = "none";
    }
    window.onclick = function(event) {
        if (event.target == modalHelp) {
            modalHelp.style.display = "none";
        }
    }
    /*END HELP MODAL CODE*/

    document.getElementById("clear").onclick = function () {
        $("#status").val(""); 
    }

    function keepStatusScrolledDown () {
        document.getElementById("status").scrollTop = document.getElementById("status").scrollHeight;  
    }

     $('#login').submit(function(e) { 
        
        e.preventDefault();

        let JSON = '{"username" : "' + $("#username").val() + '", "password" : "' + $("#password").val() + '", "db" : "' + $("#db-name").val() + '"}'; 

        $.ajax({
            type: 'get',
            dataType: 'JSON',
            url: '/login/?JSON=' + JSON,
            success: function(data) {
                console.log(data[0]);
            },
            fail: function(error) {
                $("#status").val("Error loading files from uploads directory.\n"); 
                keepStatusScrolledDown(); 
            },
            complete: function(data) {
                $("#status").val($("#status").val() + data.responseText + "\n"); 
                console.log(data.responseText); 
                if (data.responseText != "Error") {
                    modal.style.display = "none";
                } else {
                    document.getElementById("modal-status").innerHTML = "Status: Invalid credentials (" + index + ")."; 
                }
                index++;
                keepStatusScrolledDown(); 
                reload(); 
            }
        });
    });

    $('#load-db').submit(function(e) { 
        
        e.preventDefault();

        $.ajax({
            type: 'get',
            dataType: 'JSON',
            url: '/storeFiles/?JSONFiles=' + JSON.stringify(fileTable),
            success: function(data) {
                
                document.getElementById("load-db-status").innerHTML = "Status: " + data[0];  

                $.ajax({
                    type: 'get',
                    dataType: 'JSON',
                    url: '/storeFilesIndividual/?JSONIndividuals=' + JSON.stringify(individualTable),
                    success: function(data) {
                        console.log(data);
                    },
                    fail: function(error) {
                        $("#status").val("Error loading files from uploads directory.\n"); 
                        keepStatusScrolledDown(); 
                    },
                    complete: function(data) {
                        $("#status").val($("#status").val() + data.responseText + "\n"); 
                        keepStatusScrolledDown(); 
                        reload(); 
                    }
                });
            },
            fail: function(error) {
                $("#status").val("Error loading files from uploads directory.\n"); 
                keepStatusScrolledDown(); 
            },
            complete: function(data) { 
                $("#status").val($("#status").val() + data.responseText + "\n"); 
                keepStatusScrolledDown(); 
                reload(); 
            }
        });

    });

    $('#clear-db').submit(function(e) { 
    
        e.preventDefault();

        $.ajax({
            type: 'get',
            dataType: 'JSON',
            url: '/deleteRows',
            success: function(data) {
            },
            fail: function(error) {
                $("#status").val("Error clearing database.\n"); 
                document.getElementById("clear-db-status").innerHTML = "Status: Error clearing database.";  
                keepStatusScrolledDown(); 
            },
            complete: function(data) { 
                $("#status").val($("#status").val() + "Successfully cleared all rows." + "\n"); 
                document.getElementById("clear-db-status").innerHTML = "Status: Successfully cleared all rows.";  
                keepStatusScrolledDown(); 
                reload(); 
            }
        });

    });

    $('#db-status').submit(function(e) { 
    
        e.preventDefault();

        $.ajax({
            type: 'get',
            dataType: 'JSON',
            url: '/countRows',
            success: function(data) {
            },
            fail: function(error) {
                $("#status").val("Error counting rows.\n"); 
                document.getElementById("clear-db-status").innerHTML = "Status: Error counting.";  
                keepStatusScrolledDown(); 
            },
            complete: function(data) { 
                $("#status").val($("#status").val() + data.responseText + "\n"); 
                document.getElementById("db-status-status").innerHTML = "Status: " + data.responseText;  
                keepStatusScrolledDown(); 
                reload(); 
            }
        });

    });

    $('#custom-query').submit(function(e) { 
    
        e.preventDefault();

        let queryType = 0;
        queryType = $("#getQuery").val();

        if (document.getElementById("query-table").rows.length > 1) {
            for(var i = document.getElementById("query-table").rows.length; i > 1; i--) {
                document.getElementById("query-table").deleteRow(i -1);
            }
        }

        let givenName = $('#custom-query-given').val();
        let surname = $('#custom-query-surname').val();

        if (queryType == 1) {
            
            $.ajax({
                type: 'get',
                dataType: 'JSON',
                url: '/sortLastName',
                success: function(data) {
                },
                fail: function(error) {
                    $("#status").val("Error counting rows.\n"); 
                    document.getElementById("clear-db-status").innerHTML = "Status: Error perfoming query.";  
                    keepStatusScrolledDown(); 
                },
                complete: function(data) { 
                    $("#status").val($("#status").val() + "Displaying query results.\n"); 
                    document.getElementById("db-status-status").innerHTML = "Status: Dispalying results."; 

                    if (data.responseJSON == []) {
                        document.getElementById("query-status").innerHTML = "Status: Query was invalid or returned no results.";
                        return;
                    } 

                    document.getElementById("query-status").innerHTML = "Status: Displaying results."; 

                    for (dataObject of data.responseJSON) {
                        $('#query-table tr:last').after('<tr><th>' + dataObject.given_name + '</th><th>' + dataObject.surname + '</th><th>' + dataObject.sex + '</th><th>' + dataObject.fam_size + '</th><th>' + dataObject.source_file + '</th></tr>'); 
                    }

                    keepStatusScrolledDown(); 
                    reload(); 
                }
            });
       
        } else if (queryType == 2) {

            let filename = $("#getFileDatabase").val(); 

            if (!filename) {
                $("#status").val("Invalid filename.\n"); 
                 document.getElementById("query-status").innerHTML = "Status: Invalid file."; 
                 keepStatusScrolledDown();
                 return;
            }

            if (!filename.includes(".ged")) {
                $("#status").val("Invalid filename.\n"); 
                document.getElementById("query-status").innerHTML = "Status: Invalid file."; 
                keepStatusScrolledDown(); 
                return; 
            } 

            let filenameNoPath = filename.substring(8, filename.length); 

            $.ajax({
                type: 'get',
                dataType: 'JSON',
                url: '/sortIndividualsFile/?filename=' + filenameNoPath,
                success: function(data) {
                },
                fail: function(error) {
                    $("#status").val("Error counting rows.\n"); 
                    document.getElementById("clear-db-status").innerHTML = "Status: Error perfoming query.";  
                    keepStatusScrolledDown(); 
                },
                complete: function(data) { 
                    $("#status").val($("#status").val() + "Displaying query results.\n"); 
                    document.getElementById("db-status-status").innerHTML = "Status: Dispalying results."; 

                    if (data.responseJSON == []) {
                        document.getElementById("query-status").innerHTML = "Status: Query was invalid or returned no results.";
                        return;
                    } 

                    document.getElementById("query-status").innerHTML = "Status: Displaying results."; 

                    for (dataObject of data.responseJSON) {
                        $('#query-table tr:last').after('<tr><th>' + dataObject.given_name + '</th><th>' + dataObject.surname + '</th><th>' + dataObject.sex + '</th><th>' + dataObject.fam_size + '</th><th>' + dataObject.source_file + '</th></tr>'); 
                    }

                    keepStatusScrolledDown(); 
                    reload(); 
                }
            });
        
        } else if (queryType == 3) {

            $.ajax({
                type: 'get',
                dataType: 'JSON',
                url: '/sortGender',
                success: function(data) {
                },
                fail: function(error) {
                    $("#status").val("Error counting rows.\n"); 
                    document.getElementById("clear-db-status").innerHTML = "Status: Error perfoming query.";  
                    keepStatusScrolledDown(); 
                },
                complete: function(data) { 
                    $("#status").val($("#status").val() + "Displaying query results.\n"); 
                    document.getElementById("db-status-status").innerHTML = "Status: Dispalying results."; 

                    if (data.responseJSON == []) {
                        document.getElementById("query-status").innerHTML = "Status: Query was invalid or returned no results.";
                        return;
                    } 

                    document.getElementById("query-status").innerHTML = "Status: Displaying results."; 

                    for (dataObject of data.responseJSON) {
                        $('#query-table tr:last').after('<tr><th>' + dataObject.given_name + '</th><th>' + dataObject.surname + '</th><th>' + dataObject.sex + '</th><th>' + dataObject.fam_size + '</th><th>' + dataObject.source_file + '</th></tr>'); 
                    }

                    keepStatusScrolledDown(); 
                    reload(); 
                }
            });
        
        } else if (queryType == 4) {

            $.ajax({
                type: 'get',
                dataType: 'JSON',
                url: '/sortGiven/?givenName=' + givenName,
                success: function(data) {
                },
                fail: function(error) {
                    $("#status").val("Error counting rows.\n"); 
                    document.getElementById("clear-db-status").innerHTML = "Status: Error perfoming query.";  
                    keepStatusScrolledDown(); 
                },
                complete: function(data) { 
                    $("#status").val($("#status").val() + "Displaying query results.\n"); 
                    document.getElementById("db-status-status").innerHTML = "Status: Dispalying results."; 

                    if (data.responseJSON == []) {
                        document.getElementById("query-status").innerHTML = "Status: Query was invalid or returned no results.";
                        return;
                    } 

                    document.getElementById("query-status").innerHTML = "Status: Displaying results."; 

                    for (dataObject of data.responseJSON) {
                        $('#query-table tr:last').after('<tr><th>' + dataObject.given_name + '</th><th>' + dataObject.surname + '</th><th>' + dataObject.sex + '</th><th>' + dataObject.fam_size + '</th><th>' + dataObject.source_file + '</th></tr>'); 
                    }

                    keepStatusScrolledDown(); 
                    reload(); 
                }
            });

        } else if (queryType == 5) {

            $.ajax({
                type: 'get',
                dataType: 'JSON',
                url: '/sortSurname/?surname=' + surname,
                success: function(data) {
                },
                fail: function(error) {
                    $("#status").val("Error counting rows.\n"); 
                    document.getElementById("clear-db-status").innerHTML = "Status: Error perfoming query.";  
                    keepStatusScrolledDown(); 
                },
                complete: function(data) { 
                    $("#status").val($("#status").val() + "Displaying query results.\n"); 
                    document.getElementById("db-status-status").innerHTML = "Status: Dispalying results."; 

                    if (data.responseJSON == []) {
                        document.getElementById("query-status").innerHTML = "Status: Query was invalid or returned no results.";
                        return;
                    } 

                    document.getElementById("query-status").innerHTML = "Status: Displaying results."; 

                    for (dataObject of data.responseJSON) {
                        $('#query-table tr:last').after('<tr><th>' + dataObject.given_name + '</th><th>' + dataObject.surname + '</th><th>' + dataObject.sex + '</th><th>' + dataObject.fam_size + '</th><th>' + dataObject.source_file + '</th></tr>'); 
                    }

                    keepStatusScrolledDown(); 
                    reload(); 
                }
            });

        } else {
            document.getElementById("db-status-status").innerHTML = "Status: Please select a valid option."; 
        }
    
    });

    $('#custom-query-field').submit(function(e) { 
        
        e.preventDefault();

        let queryValue = $('#custom-query-input').val();

        if (document.getElementById("query-table").rows.length > 1) {
            for(var i = document.getElementById("query-table").rows.length; i > 1; i--) {
                document.getElementById("query-table").deleteRow(i -1);
            }
        }
        
        $.ajax({
            type: 'get',
            dataType: 'JSON',
            url: '/customQuery/?customQueryString=' + queryValue,
            success: function(data) {
            },
            fail: function(error) {
                $("#status").val("Error loading files from uploads directory.\n"); 
                keepStatusScrolledDown(); 
            },
            complete: function(data) { 
                $("#status").val($("#status").val() + "Displaying query results.\n"); 
                document.getElementById("db-status-status").innerHTML = "Status: Dispalying results.";  

                if (data.responseJSON == []) {
                    document.getElementById("query-status").innerHTML = "Status: Query was invalid or returned no results.";
                    return;
                } 

                document.getElementById("query-status").innerHTML = "Status: Displaying results.";

                for (dataObject of data.responseJSON) {
                    $('#query-table tr:last').after('<tr><th>' + dataObject.given_name + '</th><th>' + dataObject.surname + '</th><th>' + dataObject.sex + '</th><th>' + dataObject.fam_size + '</th><th>' + dataObject.source_file + '</th></tr>');
                }

                keepStatusScrolledDown(); 
                reload(); 
            }
        });

    });

    function reload () {

        if (document.getElementById("flog").rows.length > 1) {
            for(var i = document.getElementById("flog").rows.length; i > 1; i--) {
                document.getElementById("flog").deleteRow(i -1);
            }
        }

        $('#getIndividual').empty(); 
        $('#getIndividual').append(
            $('<option value="" selected disabled hidden>Select a File</option>')
        );

        $('#getFile').empty(); 
        $('#getFile').append(
            $('<option value="" selected disabled hidden>Select a File</option>')
        );

        $('#getFileDescendants').empty(); 
        $('#getFileDescendants').append(
            $('<option value="" selected disabled hidden>Select a File</option>')
        );

        $('#getFileAncestors').empty(); 
        $('#getFileAncestors').append(
            $('<option value="" selected disabled hidden>Select a File</option>')
        );

        $('#getFileDatabase').empty(); 
        $('#getFileDatabase').append(
            $('<option value="" selected disabled hidden>Select a File (Only for option 2)</option>')
        );

        fileNames = [];
        fileTable = [];
        individualTable = [];

        $.ajax({
            type: 'get',            
            dataType: 'JSON',       
            url: '/filenames',   
            success: function (data) {

                if (data.length == 0) {
                    document.getElementById("store-files").disabled = true;
                    $('#flog tr:last').after('<tr><th>No Files</th><th></th><th></th><th></th><th></th><th></th><th></th><th></th></tr>');
                    return; 
                }  

                let href = "href="; 
                let i = 0;
                for (file of data) {

                    let json = JSON.parse(file); 
                    let fileName = json.File.substring(8, json.File.length); 

                    filenames.push(fileName); 
                    fileTable.push(json); 

                    $('#flog tr:last').after('<tr><th><a href=\'' + json.File + '\'>'+ fileName +'</a></th><th>' + json.Source + '</th><th>' +json.Version + '</th><th>' + json.Encoding + '</th><th>' + json.SubName + '</th><th>' + json.SubAdd + '</th><th>' + json.NumInds + '</th><th>' + json.NumFams + '</th></tr>');
                    
                    $('#getIndividual').append(
                        $('<option></option>').val(json.File).html(fileName)
                    );

                    $('#getFile').append(
                        $('<option></option>').val(json.File).html(fileName)
                    );

                    $('#getFileDescendants').append(
                        $('<option></option>').val(json.File).html(fileName)
                    );

                    $('#getFileAncestors').append(
                        $('<option></option>').val(json.File).html(fileName)
                    );

                     $('#getFileDatabase').append(
                        $('<option></option>').val(json.File).html(fileName)
                    );

                    $.ajax({
                        type: 'get',
                        dataType: 'JSON',
                        url: '/individualData?fileName=' + json.File,
                        success: function(data) {
                            for (dataObject of data) {
                                let string = JSON.stringify(dataObject);  
                                let length = string.length - 1;
                                string = string.substring(0, length);
                                string += ', "file": "' + fileName + '"}';
                                let JSONstring = JSON.parse(string);
                                individualTable.push(JSONstring); 
                            }
                        }
                    });

                    i++; 
                } 

                $("#status").val($('#status').val() + "Successfully loaded files in uploads directory.\n");
                document.getElementById('log-status').innerHTML = "Status: Successfully updated files"; 

            },
            fail: function(error) {
                // Non-200 return, do something with error
                document.getElementById('log-status').innerHTML = "Status: Error loading files"; 
                $("#status").val("Error loading files from uploads directory.\n"); 
            }
        });
    }

    document.getElementById("getIndividual").onchange = function () {

        let fileName = $('#getIndividual').val(); 

        if (document.getElementById("indTable").rows.length > 1) {
            for(var i = document.getElementById("indTable").rows.length; i > 1; i--) {
                document.getElementById("indTable").deleteRow(i -1);
            }
        }
 
        $.ajax({
            type: 'get',
            dataType: 'JSON',
            url: '/individualData?fileName=' + fileName,
            success: function(data) {
                let i = 0; 

                if (data.length == 0) {
                     document.getElementById('view-status').innerHTML = "No individuals";
                     return; 
                }

                for (dataObject of data) {
                    $('#indTable tr:last').after('<tr><th>' + dataObject.givenName + '</th><th>' + dataObject.surname + '</th><th>' + dataObject.sex + '</th><th>' + dataObject.familySize + '</th></tr>');
                    i++; 
                }
                $("#status").val($("#status").val() + "Displaying: " + fileName + " individuals.\n"); 
                document.getElementById('view-status').innerHTML = ("Status: Displaying: " + fileName + " individuals."); 
                keepStatusScrolledDown(); 
            },
            fail: function(error) {
                $("#status").val("Error loading individuals\n"); 
                document.getElementById('view-status').innerHTML = ("Status: Error loading individuals"); 
                keepStatusScrolledDown(); 
            }
        });
    } 

    $('#createGEDCOM').submit(function(e) { 
        
        e.preventDefault(); 

        let JSON; 
        let fileName = $("#fileName").val(); 

        for (file of filenames) {
            if (file == fileName) {
                $("#status").val($("#status").val() + "Error creating file: Filename already in use.\n"); 
                keepStatusScrolledDown(); 
                return;
            }
        }
 
        if (!fileName.includes(".ged")) {
            $("#status").val("Invalid filename.\n"); 
            keepStatusScrolledDown(); 
            return; 
        }

        let filePath = 'uploads/' + fileName; 

        JSON = '{"filename" : "' + filePath + '", "subname" : "' + $("#subname").val() + '", "subaddress" : "' + $("#subaddress").val() + '"}'; 
       
        $.ajax({
            type: 'get',
            dataType: 'JSON',
            url: '/createGEDCOM/?JSON=' + JSON,
            success: function(data) {
                console.log(data);
            },
            fail: function(error) {
                $("#status").val("Error loading files from uploads directory.\n"); 
                keepStatusScrolledDown(); 
            },
            complete: function(data) {
                $("#status").val($("#status").val() + data.responseText + "\n"); 
                keepStatusScrolledDown(); 
                reload(); 
            }
        });
    });

    $('#addIndividual').submit(function(e) { 
        
        e.preventDefault();

        let JSON; 
        let fileName = $("#getFile").val(); 

        if (!fileName.includes(".ged")) {
            $("#status").val("Invalid filename.\n"); 
            keepStatusScrolledDown(); 
            return; 
        } 

        JSON = '{"filename" : "' + fileName + '", "givenName" : "' + $("#givenName").val() + '", "surname" : "' + $("#surname").val() + '"}'; 

        $.ajax({
            type: 'get',
            dataType: 'JSON',
            url: '/addIndividual/?JSON=' + JSON,
            success: function(data) {
                console.log(data);
            },
            fail: function(error) {
                $("#status").val("Error loading files from uploads directory.\n"); 
                keepStatusScrolledDown(); 
            },
            complete: function(data) {
                $("#status").val($("#status").val() + data.responseText + "\n"); 
                keepStatusScrolledDown(); 
                reload(); 
            }
        });
    });

    $('#getDescendants').submit(function(e) { 
        
        e.preventDefault();

        let JSONstring; 
        let fileName = $("#getFileDescendants").val(); 

        if (!fileName.includes(".ged")) {
            $("#status").val("Invalid filename.\n"); 
            keepStatusScrolledDown(); 
            return; 
        } 


        if (document.getElementById("descTable").rows.length > 1) {
            for(var i = document.getElementById("descTable").rows.length; i > 1; i--) {
                document.getElementById("descTable").deleteRow(i -1);
            }
        }

        JSONstring = '{"filename" : "' + fileName + '", "givenName" : "' + $("#givenNameDesc").val() + '", "surname" : "' + $("#surnameDesc").val() + '","maxGen":"' + $("#maxGenDesc").val() + '"}';  

        $.ajax({
            type: 'get',
            dataType: 'JSON',
            url: '/getDescendants/?JSONstring=' + JSONstring,
            success: function(data) {

                for (let i = 0; i < data.length; i ++) {
                    let obj = data[i];
                    $('#descTable tr:last').after('<tr>');
                    $('#descTable tr:last').append('<th>Generation ' + (i+1) + '</th>'); 
                    for (let el of obj) {
                        $('#descTable tr:last').append('<th>' + el.givenName + ' ' + el.surname + '</th>'); 
                    }
                    $('#descTable tr:last').after('</tr>');
                }

            },
            fail: function(error) {
                $("#status").val($("#status").val()  + "Error generating list of Descendants.\n");
                keepStatusScrolledDown(); 
            }, 
            complete: function(data) {

                if (data.responseText == "Individual not found" || data.responseText == "No Descendants" || data.responseText == "Error generating results") {
                    $("#status").val($("#status").val() + data.responseText + "\n"); 
                    document.getElementById('descStatus').innerHTML = ("Status: " + data.responseText); 
                } else {
                    $("#status").val($("#status").val()  + "Successfully generated list of Descendants.\n");
                }
                keepStatusScrolledDown(); 
            }
        });
    });     

    $('#getAncestors').submit(function(e) { 
        
        e.preventDefault();

        let JSONstring; 
        let fileName = $("#getFileAncestors").val(); 

        if (!fileName.includes(".ged")) {
            $("#status").val("Invalid filename.\n"); 
            keepStatusScrolledDown(); 
            return; 
        } 


        if (document.getElementById("ancTable").rows.length > 1) {
            for(var i = document.getElementById("ancTable").rows.length; i > 1; i--) {
                document.getElementById("ancTable").deleteRow(i -1);
            }
        }

        JSONstring = '{"filename" : "' + fileName + '", "givenName" : "' + $("#givenNameAnc").val() + '", "surname" : "' + $("#surnameAnc").val() + '","maxGen":"' + $("#maxGenAnc").val() + '"}';  

        $.ajax({
            type: 'get',
            dataType: 'JSON',
            url: '/getAncestors/?JSONstring=' + JSONstring,
            success: function(data) {

                for (let i = 0; i < data.length; i ++) {
                    let obj = data[i];
                    $('#ancTable tr:last').after('<tr>');
                    $('#ancTable tr:last').append('<th>Generation ' + (i+1) + '</th>'); 
                    for (let el of obj) {
                        $('#ancTable tr:last').append('<th>' + el.givenName + ' ' + el.surname + '</th>'); 
                    }
                    $('#ancTable tr:last').after('</tr>');
                }

            },
            fail: function(error) {
                $("#status").val($("#status").val()  + "Error generating list of Ancestors.\n");
                keepStatusScrolledDown(); 
            },
            complete: function(data) {

                if (data.responseText == "Individual not found" || data.responseText == "No Ancestors" || data.responseText == "Error generating results") {
                    $("#status").val($("#status").val() + data.responseText + "\n"); 
                    document.getElementById('ancStatus').innerHTML = ("Status: " + data.responseText); 
                } else {
                    $("#status").val($("#status").val()  + "Successfully generated list of Ancestors.\n");
                }
                keepStatusScrolledDown(); 
            }

        });
    });     
});
