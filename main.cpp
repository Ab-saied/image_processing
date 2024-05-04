#include <iostream>
#include <sys/stat.h>
#include "Image_Class.h"

using namespace std;

const int NO_OF_FILTERS = 5;

// lists of options for used in print_menu() function
string MAIN_MENU[4] = {"Load new image file", "Apply filter", "Save image", "exit"};
string SAVING_OPTIONS[2] = {"Save on the same file", "Save on a new file"};
string FILTERS_LIST[NO_OF_FILTERS] = {"Grayscale", "Merge", "Brightness", "Black & Wight","Edge_detection"};

// constexpr is used to compile in building time
constexpr char FIRST_CHAR = '0';

// checking if the image file exists
bool exists(const string &name){
    struct stat buffer{};
    return (stat (name.c_str(), &buffer) == 0);
}

// Responosible for printing lists and enumerating them
void print_menu(string arr[], const int size, const string &optionname){
    cout << endl;
    for(int i = 0; i < size; i++){
        cout << i + 1 << ") " << arr[i] << endl;
    }
    cout << "\nChoose " << optionname <<" from 1 to " << size << ": ";
}

// Taking queries and checking for them
void take_query(unsigned char &query, const int range){
    cin >> query;
    query -= FIRST_CHAR;

    // if user inputs a number out of options range it will ask again
    while(query <= 0 || query > range) {
        cout << "Choose a valid option: ";
        cin >> query;
        query -= FIRST_CHAR;
    }
}

// Adding new image to work with
// Uses load istead of normal init instruction to handle exceptions without raising them
string add_image(Image &image){
    string filename;
    cout << "Enter Image name and extension (Ex: img1.jpg): ";
    cin >> filename;

    while(!exists(filename)){
        cout << "Please enter valid filename: ";
        cin >> filename;
    }
    image.loadNewImage(filename);
    return filename;
}

// Returs true if extension is supported by the program
bool check_extention(const string &s) {
    string ext;
    const int l = int(s.length());
    bool is_ext = false; // flag variable

    for(int i = 0; i < l; i++){
        if(is_ext){
            ext += s[i];
        }
        if(s[i] == '.'){
            is_ext = true;
        }
    }

    if(ext == "jpeg" || ext == "jpg" || ext == "png" || ext == "bmp") {
        return true;
    }

    return false;
}

// Saving images (Called two times in the main body)
// Utilizes print_menu() and take_query() and check_extention()
// Check for correct file extensions
void save_image(Image image, unsigned char query, const string &filename) {
    print_menu(SAVING_OPTIONS, 2, "action");
    take_query(query, 2);

    if(query == 1) {
        image.saveImage(filename);
    }
    else {
        cout << "Enter new image file name and extension\n(Extensions must be JPEG, JPG, PNG, or BMP): ";
        string newfilename;
        cin >> newfilename;
        while(!check_extention(newfilename)) {
            cout << "Enter a name with a valid extension: ";
            cin >> newfilename;
        }
        image.saveImage(newfilename);
    }
}

// Grayscale filter works by averaging the pixel
void grayscale(Image &image) {
    for (int i = 0; i < image.width; i++) {
        for (int j = 0; j < image.height; j++) {
            const unsigned char avg = (image(i, j, 0) + image(i, j, 1) + image(i, j, 2)) / 3;

            for (int k = 0; k < 3; k++) {
                image(i, j, k) = avg;
            }
        }
    }
}

void merge(Image &image) {
    Image image2;
    add_image(image2);
    int width = min(image.width, image2.width);
    int height = min(image.height, image2.height);
    Image image3(width, height);
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            for (int k = 0; k < 3; k++) {
                image3(i, j, k) = (image(i, j, k) + image2(i, j, k)) / 2;
            }
        }
    }
    image = image3;
}

void brightness(Image &image) {

    cout << "By how % you want to change brighness?\n(More than 50% brightens and less than 50% darkens):  ";
    float brightning;
    cin >> brightning;
    while(brightning < 0 || brightning > 100) {
        cout << "Enter a valid percentage: ";
        cin >> brightning;
    }
    brightning /= 50;
    for (int i = 0; i < image.width; i++) {
        for (int j = 0; j < image.height; j++) {
            image(i, j, 0) = max(min((int)(image(i, j, 0) * brightning), 255), 0);
            image(i, j, 1) = max(min((int)(image(i, j, 1) * brightning), 255), 0);
            image(i, j, 2) = max(min((int)(image(i, j, 2) * brightning), 255), 0);
        }
    }
}

void BW(Image &image) {
    grayscale(image);
    for (int i = 0; i < image.width; i++) {
        for (int j = 0; j < image.height; j++) {
            if(image(i, j, 0) <= 127) {
                image(i, j, 0) = 0;
                image(i, j, 1) = 0;
                image(i, j, 2) = 0;
            } else {
                image(i, j, 0) = 255;
                image(i, j, 1) = 255;
                image(i, j, 2) = 255;
            }
        }
    }
}

void edge_detection(Image &image) {
    BW(image);
    Image image2(image.width, image.height);
    for (int i = 0; i < image.width; i++) {
        for (int j = 0; j < image.height; j++) {
            image2(i, j, 0) = 255;
            image2(i, j, 1) = 255;
            image2(i, j, 2) = 255;
        }
    }
    for (int i = 1; i < image.width - 1; i++) {
        for (int j = 1; j < image.height - 1; j++) {
            if( image(i-1, j-1, 0) != image(i+1, j+1, 0) ||
                image(i+1, j+1, 0) != image(i-1, j-1, 0) ||
                image(i-1, j,   0) != image(i+1, j,   0) ||
                image(i,   j-1, 0) != image(i,   j+1, 0)) {

                image2(i, j, 0) = 0;
                image2(i, j, 1) = 0;
                image2(i, j, 2) = 0;
            }
        }
    }
    image = image2;
}

// calls filters depends on the query intered by user to for cleaner main body code
void apply_filter(Image &image, const int filter_ID) {
    if(filter_ID == 1) {
        grayscale(image);
    } else if(filter_ID == 2) {
        merge(image);
    } else if(filter_ID == 3) {
        brightness(image);
    } else if(filter_ID == 4) {
        BW(image);
    }else if(filter_ID == 5) {
        edge_detection(image);
    }
}

int main() {

    bool main_flag = true;

    Image image;
    string filename = add_image(image);

    while(main_flag){

        print_menu(MAIN_MENU, 4, "action");

        unsigned char query;
        take_query(query, 4);

        if(query == 1){
            filename = add_image(image);
        }
        else if(query == 2){
            print_menu(FILTERS_LIST, NO_OF_FILTERS, "filter");
            take_query(query, 12);
            apply_filter(image, query);
        }
        else if(query == 3){
            save_image(image, query, filename);
        }
        else if(query == 4) {
            main_flag = false;
            string answers[2] = {"YES", "NO"};
            cout << "Do you want to save before exit?";
            print_menu(answers, 2, "answer");
            take_query(query, 2);
            if(query == 1) save_image(image, query, filename);
        }

        cout << endl << "-------------------------------" << endl;

    }
}
