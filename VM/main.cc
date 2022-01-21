/**
 *        @file: main.cc
 *      @author: Ethan Dowalter
 *        @date: November 15, 2021
 *       @brief: Program which emulates the way operating systems handle virtual memory
 */

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <iomanip>
using namespace std;

/// Global variables
bool DEBUG = false;             // flag for debugging code
int NUM_FRAMES = 0;             // number of frames in frame table
int NUM_PAGES = 0;              // number of pages in page table
int PAGES_REFERENCED = 0;       // number of pages referenced
int PAGES_EVER_MAPPED = 0;      // number of pages that have ever been mapped
int PAGE_MISSES = 0;            // number of page misses
int FRAMES_TAKEN = 0;           // number of times a frame has been overwritten
int FRAMES_WROTE_TO_DISK = 0;   // number of frames wrote to disk
int FRAMES_REC_FROM_DISK = 0;   // number of frames recovered from disk
int LINES = 0;                  // line number counter

struct frametable{
    bool inuse;
    bool dirty;
    int first_use;
    int last_use;
    int frequency;

    // Default Constructor
    frametable(){
        inuse = false;
        dirty = false;
        first_use = -1;
        last_use = -1;
        frequency = -1;
    }
};

struct pagetable{
    int type;       // 0 = "UnMapped", 1 = "MAPPED", 2 = "Taken"
    bool ondisk;
    int framenum;

    // Default Constructor
    pagetable(){
        type = 0;
        ondisk = false;
        framenum = -1;
    }
};

/// Print - prints out the given frametable and pagetable in the specified format, as well as the other pertinent global variables
void Print(struct frametable frametable[], struct pagetable pagetable[]);

/// Fifo - Applies the FIFO strategy on the frametable, i.e. overwrites the oldest frame
int Fifo(struct frametable frametable[], struct pagetable pagetable[], char ch);

/// Lru - Applies the LRU strategy on the frametable, i.e. overwrites the least recently used frame
int Lru(struct frametable frametable[], struct pagetable pagetable[], char ch);

/// Optimal - Applies the optimal (MIN) strategy on the frametable, i.e. overwrites the frame whose current page will be used the furthest in the future
int Optimal(struct frametable frametable[], struct pagetable pagetable[], char ch, char *file, int file_offset);

/// Prophesize - Takes the pages currently MAPPED and looks into the future until it finds the one it will need furthest in the future
int Prophesize(int pages[], char *file, int file_offset);



int main(int argc, char *argv[]){
    /// Check for valid arguments
    if (argc != 3)
    {
        cout << "Invalid number of arguments!!" << endl;
        cout << "Input should be of the form : ./vm FIFO|LRU|OPTIMAL|MIN inputfile" << endl;
        exit(1);
    }
    string strategy = argv[1];
    if (strategy != "FIFO" && strategy != "LRU" && strategy != "OPTIMAL" && strategy != "MIN")
    {
        cout << "Invalid strategy!!" << endl;
        cout << "Valid strategies are : FIFO, LRU, OPTIMAL, MIN" << endl;
        exit(1);
    }

    /// Check for valid input file
    ifstream myfile (argv[2]);
    if (!myfile)
    {
        cout << "Invalid input file!!" << endl;
        exit(1);
    }
    
    /// Get number of frames and pages from input file
    string line;            //for reading in one line from input file at a time
    while (myfile)
    {
        getline(myfile, line);
        LINES++;
        if (DEBUG) cout << LINES << ' ' << line << endl;
        if (line[0] == '#' || line == "")
            {   /// Ignore comments and empty lines

        }
        else
        {
            if (NUM_FRAMES == 0)
            {
                NUM_FRAMES = stoi(line);
            }
            else
            {
                NUM_PAGES = stoi(line);
                break;
            }
        }
    }

    /// Print the initial conditions
    cout << "Num frames: " << NUM_FRAMES << endl;
    cout << "Num pages: " << NUM_PAGES << endl;
    cout << "Reclaim algorithm: " << strategy << endl;

    /// Declare frame and page tables
    struct frametable frametable[NUM_FRAMES];
    struct pagetable pagetable[NUM_PAGES];

    /// Check the reclaim strategy and then read the rest of the input file
    int page_num;       // for reading in the page number from file
    if (strategy == "FIFO")
    {
        while (myfile)
        {
            getline(myfile, line);
            LINES++;
            /// Handle all special cases
            if (line[0] == '#' || line == "")
            {   /// Ignore comments and empty lines

            }
            else if (line == "debug")
            {   /// Turn on debugging info
                DEBUG = true;
            }
            else if (line == "nodebug")
            {   /// Turn off debugging info
                DEBUG = false;
            }
            else if (line == "print")
            {   /// Print
                Print(frametable, pagetable);
            }
            else if (line[0] == 'r' || line[0] == 'w')
            {   /// Emulate a read or write on page 'page_num'
                PAGES_REFERENCED++;
                page_num = stoi(line.substr(1));
                switch (pagetable[page_num].type)
                {
                    case 0:     // UnMapped
                        PAGES_EVER_MAPPED++;
                        PAGE_MISSES++;
                        pagetable[page_num].type = 1;
                        pagetable[page_num].framenum = Fifo(frametable, pagetable, line[0]);
                        break;
                    case 1:     // MAPPED
                        frametable[pagetable[page_num].framenum].last_use = LINES;
                        break;
                    case 2:     // Taken
                        PAGE_MISSES++;
                        if (pagetable[page_num].ondisk) FRAMES_REC_FROM_DISK++;
                        pagetable[page_num].type = 1;
                        pagetable[page_num].framenum = Fifo(frametable, pagetable, line[0]);
                        break;
                    default:
                        cout << "How did I get here??" << endl;
                }
            }
            else
            {
                cout << "Syntax error while reading file!!!" << endl;
                exit(1);
            }

            if (DEBUG) cout << LINES << ' ' << line << endl;
        }
    }
    else if (strategy == "LRU")
    {
        while (myfile)
        {
            getline(myfile, line);
            LINES++;
            /// Handle all special cases
            if (line[0] == '#' || line == "")
            {   /// Ignore comments and empty lines

            }
            else if (line == "debug")
            {   /// Turn on debugging info
                DEBUG = true;
            }
            else if (line == "nodebug")
            {   /// Turn off debugging info
                DEBUG = false;
            }
            else if (line == "print")
            {   /// Print
                Print(frametable, pagetable);
            }
            else if (line[0] == 'r' || line[0] == 'w')
            {   /// Emulate a read or write on page 'page_num'
                PAGES_REFERENCED++;
                page_num = stoi(line.substr(1));
                switch (pagetable[page_num].type)
                {
                    case 0:     // UnMapped
                        PAGES_EVER_MAPPED++;
                        PAGE_MISSES++;
                        pagetable[page_num].type = 1;
                        pagetable[page_num].framenum = Lru(frametable, pagetable, line[0]);
                        break;
                    case 1:     // MAPPED
                        frametable[pagetable[page_num].framenum].last_use = LINES;
                        break;
                    case 2:     // Taken
                        PAGE_MISSES++;
                        if (pagetable[page_num].ondisk) FRAMES_REC_FROM_DISK++;
                        pagetable[page_num].type = 1;
                        pagetable[page_num].framenum = Lru(frametable, pagetable, line[0]);
                        break;
                    default:
                        cout << "How did I get here??" << endl;
                }
            }
            else
            {
                cout << "Syntax error while reading file!!!" << endl;
                exit(1);
            }

            if (DEBUG) cout << LINES << ' ' << line << endl;
        }
    }
    else if (strategy == "OPTIMAL" || strategy == "MIN")
    {
        while (myfile)
        {
            getline(myfile, line);
            LINES++;
            /// Handle all special cases
            if (line[0] == '#' || line == "")
            {   /// Ignore comments and empty lines

            }
            else if (line == "debug")
            {   /// Turn on debugging info
                DEBUG = true;
            }
            else if (line == "nodebug")
            {   /// Turn off debugging info
                DEBUG = false;
            }
            else if (line == "print")
            {   /// Print
                Print(frametable, pagetable);
            }
            else if (line[0] == 'r' || line[0] == 'w')
            {   /// Emulate a read or write on page 'page_num'
                PAGES_REFERENCED++;
                page_num = stoi(line.substr(1));
                switch (pagetable[page_num].type)
                {
                    case 0:     // UnMapped
                        PAGES_EVER_MAPPED++;
                        PAGE_MISSES++;
                        pagetable[page_num].framenum = Optimal(frametable, pagetable, line[0], argv[2], myfile.tellg());
                        pagetable[page_num].type = 1;
                        if (pagetable[page_num].framenum == -1) cout << "Something went wrong!" << endl;
                        break;
                    case 1:     // MAPPED
                        frametable[pagetable[page_num].framenum].last_use = LINES;
                        break;
                    case 2:     // Taken
                        PAGE_MISSES++;
                        if (pagetable[page_num].ondisk) FRAMES_REC_FROM_DISK++;
                        pagetable[page_num].framenum = Optimal(frametable, pagetable, line[0], argv[2], myfile.tellg());
                        pagetable[page_num].type = 1;
                        if (pagetable[page_num].framenum == -1) cout << "Something went wrong!" << endl;
                        break;
                    default:
                        cout << "How did I get here??" << endl;
                }
            }
            else
            {
                cout << "Syntax error while reading file!!!" << endl;
                exit(1);
            }

            if (DEBUG) cout << LINES << ' ' << line << endl;
        }
    }
    else
    {
        cout << "I really shouldn't be here..." << endl;
        exit(1);
    }
    
    
    /// Don't forget to close file
    myfile.close();

    Print(frametable, pagetable);
    
    return 0;
}



void Print(struct frametable frametable[], struct pagetable pagetable[]){
    /// Print the frame table
    cout << "Frames" << endl;
    for (int i = 0; i < NUM_FRAMES; i++)
    {
        cout << setw(4) << i << setw(10) << "inuse:";
        if (frametable[i].inuse) cout << '1';
        else cout << '0';
        cout << setw(10) << "dirty:";
        if (frametable[i].dirty) cout << '1';
        else cout << '0';
        cout << setw(14) << "first_use:" << frametable[i].first_use;
        cout << setw(13) << "last_use:" << frametable[i].last_use << endl;
    }
    
    /// Print the page table
    cout << "Pages" << endl;
    for (int i = 0; i < NUM_PAGES; i++)
    {
        cout << setw(4) << i << setw(9) << "type:";
        if (pagetable[i].type == 0) cout << setw(8) << "UnMapped";
        else if (pagetable[i].type == 1) cout << setw(8) << "MAPPED";
        else if (pagetable[i].type == 2) cout << setw(8) << "Taken";
        else cout << "I shouldn't be here!";
        cout << setw(11) << "ondisk:";
        if (pagetable[i].ondisk) cout << '1';
        else cout << '0';
        cout << setw(13) << "framenum:";
        if (pagetable[i].framenum == -1) cout << "(unassigned)" << endl;
        else cout << pagetable[i].framenum << endl;
    }

    /// Print the global variables
    cout << "Pages referenced: " << PAGES_REFERENCED << endl;
    cout << "Pages ever mapped: " << PAGES_EVER_MAPPED << endl;
    cout << "Page misses: " << PAGE_MISSES << endl;
    cout << "Frames taken: " << FRAMES_TAKEN << endl;
    cout << "Frames written to disk: " << FRAMES_WROTE_TO_DISK << endl;
    cout << "Frames recovered from disk: " << FRAMES_REC_FROM_DISK << endl;
}

int Fifo(struct frametable frametable[], struct pagetable pagetable[], char ch){
    /// Convert 'ch' value of either 'r' or 'w' to bool value
    bool is_write = false;
    if (ch == 'w') is_write = true;

    /// Check for empty frames first
    for (int i = 0; i < NUM_FRAMES; i++)
    {
        if (!frametable[i].inuse)
        {
            frametable[i].inuse = true;
            frametable[i].first_use = LINES;
            frametable[i].last_use = LINES;
            if (is_write) frametable[i].dirty = true;
            return i;
        }
    }

    /// Find frame which has been in table the longest (first in)
    int earliest_use = frametable[0].first_use;
    int oldest_frame = 0;
    for (int i = 1; i < NUM_FRAMES; i++)
    {
        if (frametable[i].first_use < earliest_use)
        {
            earliest_use = frametable[i].first_use;
            oldest_frame = i;
        }
    }
    
    /// Check if oldest frame was dirty
    if (frametable[oldest_frame].dirty)
    {
        for (int i = 0; i < NUM_PAGES; i++)
        {
            if (pagetable[i].framenum == oldest_frame)
            {
                FRAMES_WROTE_TO_DISK++;
                pagetable[i].ondisk = true;
                break;
            }
        }
    }

    /// Update frame with new info
    frametable[oldest_frame].first_use = LINES;
    frametable[oldest_frame].last_use = LINES;
    if (is_write) frametable[oldest_frame].dirty = true;
    else frametable[oldest_frame].dirty = false;

    /// Unmap oldest frame from page table
    for (int i = 0; i < NUM_PAGES; i++)
    {
        if (pagetable[i].framenum == oldest_frame)
        {
            FRAMES_TAKEN++;
            pagetable[i].type = 2;
            pagetable[i].framenum = -1;
        }
    }
    
    /// Return index of oldest frame
    return oldest_frame;
}

int Lru(struct frametable frametable[], struct pagetable pagetable[], char ch){
    /// Convert 'ch' value of either 'r' or 'w' to bool value
    bool is_write = false;
    if (ch == 'w') is_write = true;

    /// Check for empty frames first
    for (int i = 0; i < NUM_FRAMES; i++)
    {
        if (!frametable[i].inuse)
        {
            frametable[i].inuse = true;
            frametable[i].first_use = LINES;
            frametable[i].last_use = LINES;
            if (is_write) frametable[i].dirty = true;
            return i;
        }
    }

    /// Find frame which has been least recently used
    int earliest_use = frametable[0].last_use;
    int oldest_frame = 0;
    for (int i = 1; i < NUM_FRAMES; i++)
    {
        if (frametable[i].last_use < earliest_use)
        {
            earliest_use = frametable[i].last_use;
            oldest_frame = i;
        }
    }
    
    /// Check if oldest frame was dirty
    if (frametable[oldest_frame].dirty)
    {
        for (int i = 0; i < NUM_PAGES; i++)
        {
            if (pagetable[i].framenum == oldest_frame)
            {
                FRAMES_WROTE_TO_DISK++;
                pagetable[i].ondisk = true;
                break;
            }
        }
    }

    /// Update frame with new info
    frametable[oldest_frame].first_use = LINES;
    frametable[oldest_frame].last_use = LINES;
    if (is_write) frametable[oldest_frame].dirty = true;
    else frametable[oldest_frame].dirty = false;

    /// Unmap oldest frame from page table
    for (int i = 0; i < NUM_PAGES; i++)
    {
        if (pagetable[i].framenum == oldest_frame)
        {
            FRAMES_TAKEN++;
            pagetable[i].type = 2;
            pagetable[i].framenum = -1;
        }
    }
    
    /// Return index of oldest frame
    return oldest_frame;
}

int Optimal(struct frametable frametable[], struct pagetable pagetable[], char ch, char *file, int file_offset){
    /// Convert 'ch' value of either 'r' or 'w' to bool value
    bool is_write = false;
    if (ch == 'w') is_write = true;

    /// Check for empty frames first
    for (int i = 0; i < NUM_FRAMES; i++)
    {
        if (!frametable[i].inuse)
        {
            frametable[i].inuse = true;
            frametable[i].first_use = LINES;
            frametable[i].last_use = LINES;
            if (is_write) frametable[i].dirty = true;
            return i;
        }
    }
    
    if (DEBUG)
    {   /// Print status of pagetable
        for (int i = 0; i < NUM_PAGES; i++)
        {
            cout << pagetable[i].type << " ";
        }
        cout << endl;
    }

    /// Find page that is in frame table which will be used the furthest in the future
    /// Do this by first gathering which pages are MAPPED
    int pages[NUM_FRAMES];
    int j = 0;
    for (int i = 0; i < NUM_PAGES; i++)
    {
        if (pagetable[i].type == 1)
        {
            pages[j] = i;
            if (DEBUG) cout << pages[j] << " ";
            j++;
            if (j == NUM_FRAMES) break;
        }
    }
    if (DEBUG) cout << endl;
    
    /// Then, read 'file' starting at 'file_offset' and see which frame that it is
    int page_prophecy = Prophesize(pages, file, file_offset);
    int frame_prophecy = pagetable[page_prophecy].framenum;

    /// Check if prophecized frame was dirty
    if (frametable[frame_prophecy].dirty)
    {
        for (int i = 0; i < NUM_PAGES; i++)
        {
            if (pagetable[i].framenum == frame_prophecy)
            {
                FRAMES_WROTE_TO_DISK++;
                pagetable[i].ondisk = true;
                break;
            }
        }
    }

    /// Update the prophecized frame with new info
    frametable[frame_prophecy].first_use = LINES;
    frametable[frame_prophecy].last_use = LINES;
    if (is_write) frametable[frame_prophecy].dirty = true;
    else frametable[frame_prophecy].dirty = false;

    /// Unmap the old frame from page table
    FRAMES_TAKEN++;
    pagetable[page_prophecy].type = 2;
    pagetable[page_prophecy].framenum = -1;
    
    /// Return the prophecy
    return frame_prophecy;
}

int Prophesize(int pages[], char *file, int file_offset){
    /// Open 'file' at 'file_offset'
    ifstream prognosticating (file);
    prognosticating.seekg(file_offset);

    /// Make an array of 'found' statuses that corresponds to the 'pages' array
    bool found[NUM_FRAMES];
    for (int i = 0; i < NUM_FRAMES; i++)
    {
        found[i] = false;
    }
    
    /// Read through the file until the prophecy has been found
    string line;
    int page_num;
    bool all_found;
    while (prognosticating)
    {
        getline(prognosticating, line);
        if (line[0] == '#' || line == "" || line == "debug" || line == "nodebug" || line == "print")
        {   /// Ignore everything that isn't a read or a write

        }
        else if (line[0] == 'r' || line[0] == 'w')
        {
            page_num = stoi(line.substr(1));
            for (int i = 0; i < NUM_FRAMES; i++)
            {   /// Check to see if page_num is in pages[], if so, set found[i] = true
                if (pages[i] == page_num)
                {
                    found[i] = true;
                    break;
                }
            }
            all_found = true;   //assume it's true to begin with
            for (int i = 0; i < NUM_FRAMES; i++)
            {   /// Check to see if found[] is all true, otherwise all_found = false
                if (!found[i])
                {
                    all_found = false;
                    break;
                }
            }
            if (all_found)
            {   /// If found[] was all true, return page_num which was the last page read from the file
                /// Don't forget to close file
                prognosticating.close();
                if (DEBUG) cout << "Returning: " << page_num << endl;
                return page_num;
            }
        }
        else
        {
            cout << "Syntax error while reading file!!!" << endl;
            exit(1);
        }
    }

    /// Don't forget to close file
    prognosticating.close();

    /// If we get to the end of the file, that means there is at least one page that we will never use again
    /// Therefore, we should kick that page out of the frametable
    for (int i = 0; i < NUM_FRAMES; i++)
    {
        if (!found[i])
        {
            if (DEBUG) cout << "Got to EOF | LINES i pages[i] = " << LINES << " " << i << " " << pages[i] << endl;
            return pages[i];
        }
    }

    /// Function should never reach this point, so return an invalid value to make compiler happy
    cout << "HOW?" << endl;
    return -1;
}