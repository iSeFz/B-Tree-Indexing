#include <bits/stdc++.h>
using namespace std;

/* Global variables used throughout the program */
int fileNodes = 0;
int fileRecords = 0;

/* Mandatory functions definitions */
// Create the index file with certain number of records
void createIndexFile(char *filename, int numberOfRecords, int m)
{
    // Open the file for output in binary mode
    fstream file(filename, ios::out | ios::binary);
    file.seekp(0, ios::beg);
    // Save the number of nodes and records in the file
    fileNodes = m;
    fileRecords = numberOfRecords;
    int numberOfCells = 2 * fileNodes + 1;
    short initialCell = -1;
    // Loop to write create records with the given number of records
    for (int i = 0; i < fileRecords; i++)
    {
        // Write the initial cell value
        file.write((char *)(&initialCell), sizeof(short));
        // If its the last record, write -1 as the last pointer value
        if (i == fileRecords - 1)
            file.write((char *)(&initialCell), sizeof(short));
        else // Otherwise write the current pointer value
        {
            int nextRecordPointer = i + 1;
            file.write((char *)(&nextRecordPointer), sizeof(short));
        }
        // Write the rest of the cells of the current record
        for (int j = 0; j < numberOfCells - 2; j++)
            file.write((char *)(&initialCell), sizeof(short));
    }
    file.close();
}

// Insert a new record to the index file & return -1 if there is no place to insert the record
// Or return the index of the node where the new record is inserted if the record was inserted successfully
int insertNewRecordAtIndex(char *filename, int recordID, int reference);

// Delete a certain record from the index file by its record ID
void deleteRecordFromIndex(char *filename, int recordID);

// Display the contents of the index file, each node in a line
void displayIndexFileContent(char *filename)
{
    // Open the file for reading in binary mode
    fstream file(filename, ios::in | ios::binary);
    file.seekg(0, ios::beg);
    int numberOfCells = 2 * fileNodes + 1;
    // Loop to read all records until end of the file
    for (int i = 0; i < fileRecords; i++)
    { // Loop to read the cells of each record
        for (int j = 0; j < numberOfCells; j++)
        { // Output the current cell value
            short currentCell;
            file.read((char *)(&currentCell), sizeof(short));
            cout << currentCell << "  |  ";
        }
        cout << "\n"; // If reached the end of the record, output a new line
    }
    file.close();
}

// Search for a certain record in the index file using its record ID
// Return -1 if the record doesn’t exist in the index
// Or return the reference value to the data file if the record exist on the index
int searchARecord(char *filename, int recordID)
{
    // Open the file for reading in binary mode
    fstream file(filename, ios::in | ios::binary);
    // Size of each record = (2m + 1) cells * 2 bytes per cell
    int recordSize = (2 * fileNodes + 1) * 2;
    // Skip the first record of the index file & the first cell of the first record
    file.seekg(recordSize + 2, ios::beg);
    // Sequential search for the record ID in the index file
    while (true)
    {
        // Read the record ID of the current record
        int currentRecordID;
        file.read((char *)(&currentRecordID), sizeof(short));
        // If the current key is equal to the searched key, return its reference value
        if (currentRecordID == recordID)
        {
            short reference;
            file.read((char *)(&reference), sizeof(short));
            return reference;
        }
        // If the current key is greater than the searched key, return -1
        else if (currentRecordID > recordID)
        { // Seek to the next node
            short nextReference;
            file.read((char *)(&nextReference), sizeof(short));
            file.seekg(nextReference * recordSize + 2, ios::beg);
        }
        // Otherwise, if the current key is smaller than the searched key
        // Seek to the next item in the current node
        else
            file.seekg(2, ios::cur);
    }
    return -1; // If the record doesn't exist in the index
}

// Main function to start the program
int main()
{
    cout << "\tWelcome to the B-Tree Indexing System!";
    while (true)
    {
        cout << "\n1. Create a new index file\n"
                "2. Insert a new record\n"
                "3. Delete a record\n"
                "4. Search for a record\n"
                "5. Display the index file content\n"
                "6- Exit\n"
                "Choose one of the above options >> ";
        int choice;
        cin >> choice;
        switch (choice)
        {
        case 1: // Creating the index file from scratch
        {
            cout << "\tCreateing Index File...\n";
            cout << "Enter the number of records: ";
            cin >> fileRecords;
            cout << "Enter the number of nodes: ";
            cin >> fileNodes;
            createIndexFile("bTreeIndex.bin", fileRecords, fileNodes);
            cout << "\tIndex File Created Successfully!\n";
            break;
        }
        case 2: // Inserting a new record to the index file
        {
            cout << "\tInserting a new record to the index file...\n";
            /*
            cout << "Enter the record ID: ";
            int recordID;
            cin >> recordID;
            cout << "Enter the reference value: ";
            int reference;
            cin >> reference;
            int nodeIndex = insertNewRecordAtIndex("bTreeIndex.bin", recordID, reference);
            if (nodeIndex == -1)
                cout << "There is no place to insert the record!\n";
            else
                cout << "The record was inserted successfully at node " << nodeIndex << "\n";
            */
            break;
        }
        case 3: // Deleting a record from the index file
        {
            cout << "\tDeleting certain record from the index file...\n";
            /*
            cout << "Enter the record ID: ";
            int recordID;
            cin >> recordID;
            deleteRecordFromIndex("bTreeIndex.bin", recordID);
            */
            break;
        }
        case 4: // Searching for a record in the index file
        {
            cout << "\tSearching for a record in the index file...\n";
            cout << "Enter the record ID: ";
            int recordID;
            cin >> recordID;
            int reference = searchARecord("bTreeIndex.bin", recordID);
            if (reference == -1)
                cout << "Record doesn't exist in the index!\n";
            else
                cout << "Data file reference value ==> " << reference << "\n";
            break;
        }
        case 5: // Displaying the contents of the index file
        {
            cout << "\t\tIndex File Contents\n";
            displayIndexFileContent("bTreeIndex.bin");
            break;
        }
        case 6:
        {
            cout << "\tThanks for using our B-Tree Index Application!\n";
            return 0;
        }
        }
    }
}
