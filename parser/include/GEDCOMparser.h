#ifndef GEDCOMPARSER_H
#define GEDCOMPARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "LinkedListAPI.h"

//For simplicity, the examples we will use will only use the ASCII subset of these encodings 
typedef enum cSet {ANSEL, UTF8, UNICODE, ASCII} CharSet;

//error code enum
typedef enum eCode {OK, INV_FILE, INV_GEDCOM, INV_HEADER, INV_RECORD, OTHER_ERROR, WRITE_ERROR} ErrorCode;

//Represents a generic event, e.g. individual event, family event, etc.
typedef struct {
    //The max length of this field is known from the GEDCOM spec, so we can use a statically allocated array
    char type[5];
    
    //Empty string if not provided
    char* date;
    
    //Empty string if not provided
    char* place;
    
    //All other event fields. All objects in the list will be of type Field.  It may be empty.
    List    otherFields;
    
} Event;

//Represents a generic field.
typedef struct {
    //Field tag.  Must not be NULL/empty.
    char* tag;
    
    //Field value.  Must not be NULL/empty.
    char* value;
} Field;

//Represents a submitter record.  This is a separate type/struct, in case we decide to expand it in later assignments
typedef struct {
    //Submitter name has a max length and only appears once, so we can hardcode it
    char    submitterName[61];
    
    //All other submitter fields. All objects in the list will be of type Field.  It may be empty.
    List    otherFields;
    
    //Submitted address.  We use a C99 flexible array member, which we will discuss in class.
    char    address[];
} Submitter;

/*
 Represents the GEDCOM header
 Only includes required fields ("line values" in GEDCOM terminology)
 Note that while GEDCOM_FORM is required, but for us it will always be Lineage-Linked
 */
typedef struct {
    //Header source - i.e. software that produced the GEDCOM file
    char        source[249];
  
    //GEDCOM version
    float       gedcVersion;
    
    //Encoding.  We use an enum, since there are only 4 possible values.
    CharSet     encoding;
    
    //Reference to the submitter record
    Submitter*  submitter;
    
    //All other header fields. All objects in the list will be of type Field.  It may be empty.
    List        otherFields;
    
} Header;

//Represends GEDCOM individual record
typedef struct {
    
    //Set to empty string if not present in file
    char*    givenName;
    
    //Set to empty string if not present in file
    char*    surname;
    
    //Collection of individual events. All objects in the list will be of type Event.  It may be empty.
    List    events;
    
    //Collection of family references.  All objects in the list will be of type Family.  It may be empty.
    List    families;
    
    //All other individual record fields. All objects in the list will be of type Field.  It may be empty.
    List    otherFields;
    
} Individual;

//Represends GEDCOM family record
typedef struct {
    //Wife reference (can be null)
    Individual* wife;
    
    //Husband reference (can be null)
    Individual* husband;
    
    //List of child references.  All objects in the list will be of type Individual.  It may be empty.
    List        children;
    
    //Collection of family events. All objects in the list will be of type Event.  It may be empty.
    List        events;
    
    //List of other fields in the family record.  All objects in the list will be of type Field.  It may be empty.
    List        otherFields;
    
} Family;

//Represents a GEDCOM object
typedef struct {

	//Header.  Must not be NULL.
    Header*     header;
    
    //Family records.  All objects in the list will be of type Family.  It may be empty.
    List        families; //Must contain type
    
    //Individual records.  All objects in the list will be of type Individual.  It may be empty.
    List        individuals; //Must contain type Family
    
    //Submitter.  Must not be NULL.
    Submitter*  submitter;
    
    //All other records should be ignored for now
    
} GEDCOMobject;

//Error type
typedef struct {
    
    ErrorCode   type;
    int         line;
    
} GEDCOMerror;


//***************************************** GEDCOOM object functions *****************************************

/** Function to create a GEDCOM object based on the contents of an GEDCOM file.
 *@pre File name cannot be an empty string or NULL.  File name must have the .ged extension.
 File represented by this name must exist and must be readable.
 *@post Either:
 A valid GEDCOM has been created, its address was stored in the variable obj, and OK was returned
 or
 An error occurred, the GEDCOM was not created, all temporary memory was freed, obj was set to NULL, and the
 appropriate error code was returned
 *@return the error code indicating success or the error encountered when parsing the GEDCOM
 *@param fileName - a string containing the name of the GEDCOM file
 *@param a double pointer to a GEDCOMobject struct that needs to be allocated
 **/
GEDCOMerror createGEDCOM(char* fileName, GEDCOMobject** obj);


/** Function to create a string representation of a GEDCOMobject.
 *@pre GEDCOMobject object exists, is not null, and is valid
 *@post GEDCOMobject has not been modified in any way, and a string representing the GEDCOM contents has been created
 *@return a string contaning a humanly readable representation of a GEDCOMobject
 *@param obj - a pointer to a GEDCOMobject struct
 **/
char* printGEDCOM(const GEDCOMobject* obj);


/** Function to delete all GEDCOM object content and free all the memory.
 *@pre GEDCOM object exists, is not null, and has not been freed
 *@post GEDCOM object had been freed
 *@return none
 *@param obj - a pointer to a GEDCOMobject struct
 **/
void deleteGEDCOM(GEDCOMobject* obj);


/** Function to "convert" the GEDCOMerror into a humanly redabale string.
 *@return a string contaning a humanly readable representation of the error code
 *@param err - an error struct
 **/
char* printError(GEDCOMerror err);

/** Function that searches for an individual in the list using a comparator function.
 * If an individual is found, a pointer to the Individual record
 * Returns NULL if the individual is not found.
 *@pre GEDCOM object exists,is not NULL, and is valid.  Comparator function has been provided.
 *@post GEDCOM object remains unchanged.
 *@return The Individual record associated with the person that matches the search criteria.  If the Individual record is not found, return NULL.
 *If multiple records match the search criteria, return the first one.
 *@param familyRecord - a pointer to a GEDCOMobject struct
 *@param compare - a pointer to comparator fuction for customizing the search
 *@param person - a pointer to search data, which contains seach criteria
 *Note: while the arguments of compare() and person are all void, it is assumed that records they point to are
 *      all of the same type - just like arguments to the compare() function in the List struct
 **/
Individual* findPerson(const GEDCOMobject* familyRecord, bool (*compare)(const void* first, const void* second), const void* person);


/** Function to return a list of all descendants of an individual in a GEDCOM
 *@pre GEDCOM object exists, is not null, and is valid
 *@post GEDCOM object has not been modified in any way, and a list of descendants has been created
 *@return a list of descendants.  The list may be empty.  All list members must be of type Individual, and can appear in any order.
 *All list members must be COPIES of the Individual records in the GEDCOM file.  If the returned list is freed, the original GEDCOM
 *must remain unaffected.
 *@param familyRecord - a pointer to a GEDCOMobject struct
 *@param person - the Individual record whose descendants we want
 **/
List getDescendants(const GEDCOMobject* familyRecord, const Individual* person);



// ****************************** A2 functions ******************************

/** Function to writing a GEDCOMobject into a file in GEDCOM format.
 *@pre GEDCOMobject object exists, is not null, and is valid
 *@post GEDCOMobject has not been modified in any way, and a file representing the
 GEDCOMobject contents in GEDCOM format has been created
 *@return the error code indicating success or the error encountered when parsing the calendar
 *@param obj - a pointer to a GEDCOMobject struct
 **/
GEDCOMerror writeGEDCOM(char* fileName, const GEDCOMobject* obj);

/** Function for validating an existing GEDCOM object
 *@pre GEDCOM object exists and is not null
 *@post GEDCOM object has not been modified in any way
 *@return the error code indicating success or the error encountered when validating the GEDCOM
 *@param obj - a pointer to a GEDCOMobject struct
 **/
ErrorCode validateGEDCOM(const GEDCOMobject* obj);

/** Function to return a list of up to N generations of descendants of an individual in a GEDCOM
 *@pre GEDCOM object exists, is not null, and is valid
 *@post GEDCOM object has not been modified in any way, and a list of descendants has been created
 *@return a list of descendants.  The list may be empty.  All list members must be of type List.    *@param familyRecord - a pointer to a GEDCOMobject struct
 *@param person - the Individual record whose descendants we want
 *@param maxGen - maximum number of generations to examine (must be >= 1)
 **/
List getDescendantListN(const GEDCOMobject* familyRecord, const Individual* person, unsigned int maxGen);

/** Function to return a list of up to N generations of ancestors of an individual in a GEDCOM
 *@pre GEDCOM object exists, is not null, and is valid
 *@post GEDCOM object has not been modified in any way, and a list of ancestors has been created
 *@return a list of ancestors.  The list may be empty.
 *@param familyRecord - a pointer to a GEDCOMobject struct
 *@param person - the Individual record whose descendants we want
 *@param maxGen - maximum number of generations to examine (must be >= 1)
 **/
List getAncestorListN(const GEDCOMobject* familyRecord, const Individual* person, int maxGen);

/** Function for converting an Individual struct into a JSON string
 *@pre Individual exists, is not null, and is valid
 *@post Individual has not been modified in any way, and a JSON string has been created
 *@return newly allocated JSON string.  May be NULL.
 *@param ind - a pointer to an Individual struct
 **/
char* indToJSON(const Individual* ind);

/** Function for creating an Individual struct from an JSON string
 *@pre String is not null, and is valid
 *@post String has not been modified in any way, and an Individual struct has been created
 *@return a newly allocated Individual struct.  May be NULL.
 *@param str - a pointer to a JSON string
 **/
Individual* JSONtoInd(const char* str);

/** Function for creating a GEDCOMobject struct from an JSON string
 *@pre String is not null, and is valid
 *@post String has not been modified in any way, and a GEDCOMobject struct has been created
 *@return a newly allocated GEDCOMobject struct.  May be NULL.
 *@param str - a pointer to a JSON string
 **/
GEDCOMobject* JSONtoGEDCOM(const char* str);

/** Function for adding an Individual to a GEDCCOMobject
 *@pre both arguments are not NULL and valid
 *@post Individual has not been modified in any way, and its address had been added to GEDCOMobject's individuals list
 *@return void
 *@param obj - a pointer to a GEDCOMobject struct
 *@param toBeAdded - a pointer to an Individual struct
**/
void addIndividual(GEDCOMobject* obj, const Individual* toBeAdded);

/** Function for converting a list of Individual structs into a JSON string
 *@pre List exists, is not null, and has been initialized
 *@post List has not been modified in any way, and a JSON string has been created
 *@return newly allocated JSON string.  May be NULL.
 *@param iList - a pointer to a list of Individual structs
 **/
char* iListToJSON(List iList);

/** Function for converting a list of lists of Individual structs into a JSON string
 *@pre List exists, is not null, and has been initialized
 *@post List has not been modified in any way, and a JSON string has been created
 *@return newly allocated JSON string.  May be NULL.
 *@param gList - a pointer to a list of lists of Individual structs
 **/
char* gListToJSON(List gList);


//****************************************** List helper functions added for A2 *******************************************
void deleteGeneration(void* toBeDeleted);
int compareGenerations(const void* first,const void* second);
char* printGeneration(void* toBePrinted);

//************************************************************************************************************

//****************************************** List helper functions *******************************************
void deleteEvent(void* toBeDeleted);
int compareEvents(const void* first,const void* second);
char* printEvent(void* toBePrinted);

void deleteIndividual(void* toBeDeleted);
int compareIndividuals(const void* first,const void* second);
char* printIndividual(void* toBePrinted);

void deleteFamily(void* toBeDeleted);
int compareFamilies(const void* first,const void* second);
char* printFamily(void* toBePrinted);

void deleteField(void* toBeDeleted);
int compareFields(const void* first,const void* second);
char* printField(void* toBePrinted);


//************************************************************************************************************

#endif
