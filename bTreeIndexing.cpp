#include <bits/stdc++.h>
using namespace std;

                    /* Mandatory functions prototypes */
// Create the index file with certain number of records
void createIndexFile(char *filename, int numberOfRecords, int m);

// Insert a new record to the index file & return -1 if there is no place to insert the record
// Or return the index of the node where the new record is inserted if the record was inserted successfully
int insertNewRecordAtIndex(char *filename, int recordID, int reference);

// Delete a certain record from the index file by its record ID
void deleteRecordFromIndex(char *filename, int recordID);

// Display the contents of the index file, each node in a line
void displayIndexFileContent(char *filename);

// Search for a certain record in the index file using its record ID
// Return -1 if the record doesn’t exist in the index
// Or return the reference value to the data file if the record exist on the index
int searchARecord(char *filename, int recordID);

// Main function to start the program
int main() {
    cout << "\tWelcome to the B-Tree Indexing System!\n";
    return 0;
}