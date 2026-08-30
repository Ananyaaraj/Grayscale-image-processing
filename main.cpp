#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <stdexcept>  // For exceptions

using namespace std;

// Helper function to extract filename from full path
string extractFilename(const string& path) {
    size_t slash = path.find_last_of("/\\");
    if (slash == string::npos) return path;
    return path.substr(slash + 1);
}

// Abstract Base Class
class ImageProcessor {
protected:
    int width, height, channels;
    unsigned char* img = nullptr;
    string inputImage, outputImage, baseFilename;

public:
    virtual ~ImageProcessor() {
        if (img) {
            stbi_image_free(img);
        }
    }

    bool loadImage(const string& filename) {
        try {
            inputImage = filename;
            baseFilename = extractFilename(filename);
            img = stbi_load(inputImage.c_str(), &width, &height, &channels, 0);

            if (!img) {
                throw runtime_error("Error: Could not load image: " + inputImage);
            }
            cout << "Loaded image: " << width << "x" << height << " with " << channels << " channels.\n";
            return true;
        } catch (const exception& e) {
            cerr << e.what() << "\n";
            return false;
        }
    }

    void saveImage(const string& filename, const vector<unsigned char>& data, int comp) {
        try {
            if (!stbi_write_png(filename.c_str(), width, height, comp, data.data(), width * comp)) {
                throw runtime_error("Error: Could not save image!");
            }
            cout << "Image saved as: " << filename << "\n";

            ofstream outputFile("output.txt", ios::app);
            if (!outputFile.is_open()) {
                throw runtime_error("Error: Could not write to output.txt");
            }
            outputFile << filename << endl;
            outputFile.close();
        } catch (const exception& e) {
            cerr << e.what() << "\n";
        }
    }

    virtual void processImage() = 0;
};

// Grayscale Effect
class GrayscaleConverter : public ImageProcessor {
public:
    void processImage() override {
        try {
            outputImage = "grayscale_" + baseFilename;
            vector<unsigned char> grayData(width * height);

            for (int i = 0; i < width * height; i++) {
                int r = img[i * channels];
                int g = img[i * channels + 1];
                int b = img[i * channels + 2];
                grayData[i] = static_cast<unsigned char>(0.299 * r + 0.587 * g + 0.114 * b);
            }

            saveImage(outputImage, grayData, 1);
        } catch (const exception& e) {
            cerr << "Error processing grayscale: " << e.what() << "\n";
        }
    }
};

// Sepia Effect
class SepiaEffect : public ImageProcessor {
public:
    void processImage() override {
        try {
            outputImage = "sepia_" + baseFilename;
            vector<unsigned char> sepiaData(img, img + width * height * channels);

            for (int i = 0; i < width * height; i++) {
                int r = img[i * channels];
                int g = img[i * channels + 1];
                int b = img[i * channels + 2];

                int tr = (0.393 * r) + (0.769 * g) + (0.189 * b);
                int tg = (0.349 * r) + (0.686 * g) + (0.168 * b);
                int tb = (0.272 * r) + (0.534 * g) + (0.131 * b);

                sepiaData[i * channels] = min(255, tr);
                sepiaData[i * channels + 1] = min(255, tg);
                sepiaData[i * channels + 2] = min(255, tb);
            }

            saveImage(outputImage, sepiaData, channels);
        } catch (const exception& e) {
            cerr << "Error processing sepia: " << e.what() << "\n";
        }
    }
};

// Edge Detection
class EdgeDetection : public ImageProcessor {
public:
    void processImage() override {
        try {
            outputImage = "edge_detected_" + baseFilename;
            vector<unsigned char> edgeData(width * height, 0);

            int sobelX[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
            int sobelY[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

            for (int y = 1; y < height - 1; ++y) {
                for (int x = 1; x < width - 1; ++x) {
                    int gx = 0, gy = 0;
                    for (int ky = -1; ky <= 1; ++ky) {
                        for (int kx = -1; kx <= 1; ++kx) {
                            int pixel = img[((y + ky) * width + (x + kx)) * channels];
                            gx += sobelX[ky + 1][kx + 1] * pixel;
                            gy += sobelY[ky + 1][kx + 1] * pixel;
                        }
                    }
                    int edgeValue = min(255, static_cast<int>(sqrt(gx * gx + gy * gy)));
                    edgeData[y * width + x] = edgeValue;
                }
            }

            saveImage(outputImage, edgeData, 1);
        } catch (const exception& e) {
            cerr << "Error processing edge detection: " << e.what() << "\n";
        }
    }
};

// Invert Colors Effect
class InvertColorsEffect : public ImageProcessor {
public:
    void processImage() override {
        try {
            outputImage = "invert_" + baseFilename;
            vector<unsigned char> invertedData(img, img + width * height * channels);

            for (int i = 0; i < width * height * channels; i++) {
                invertedData[i] = 255 - img[i];
            }

            saveImage(outputImage, invertedData, channels);
        } catch (const exception& e) {
            cerr << "Error processing invert colors: " << e.what() << "\n";
        }
    }
};
// Extreme Blur Effect
class ExtremeBlurEffect : public ImageProcessor {
    public:
        void processImage() override {
            outputImage = "extreme_blur_" + baseFilename;
            vector<unsigned char> blurData(img, img + width * height * channels);
            vector<unsigned char> tempData = blurData; 
    
            int sumKernel = 25; 
    
            for (int pass = 0; pass < 3; ++pass) { 
                for (int y = 2; y < height - 2; ++y) {
                    for (int x = 2; x < width - 2; ++x) {
                        int sum[3] = {0, 0, 0};
    
                        for (int ky = -2; ky <= 2; ++ky) {
                            for (int kx = -2; kx <= 2; ++kx) {
                                for (int c = 0; c < 3; ++c) {
                                    sum[c] += blurData[((y + ky) * width + (x + kx)) * channels + c];
                                }
                            }
                        }
    
                        for (int c = 0; c < 3; ++c) {
                            tempData[(y * width + x) * channels + c] = sum[c] / sumKernel;
                        }
                    }
                }
                blurData = tempData; 
            }
    
            saveImage(outputImage, blurData, channels);
        }
    };
    class VintageEffect : public ImageProcessor {
        public:
            void processImage() override {
                outputImage = "vintage_" + baseFilename;
                vector<unsigned char> vintageData(img, img + width * height * channels);
        
                for (int i = 0; i < width * height; i++) {
                    int r = img[i * channels];
                    int g = img[i * channels + 1];
                    int b = img[i * channels + 2];
        
                    int tr = min(255, static_cast<int>(r * 0.9 + 20));
                    int tg = min(255, static_cast<int>(g * 0.85 + 15));
                    int tb = min(255, static_cast<int>(b * 0.75 + 10));
        
                    vintageData[i * channels] = tr;
                    vintageData[i * channels + 1] = tg;
                    vintageData[i * channels + 2] = tb;
                }
        
                saveImage(outputImage, vintageData, channels);
            }
        };
        
        
// Display Menu
void displayMenu() {
    cout << "\n------ Image Processing Menu ------\n";
    cout << "1. Convert to Grayscale\n";
    cout << "2. Apply Sepia Effect\n";
    cout << "3. Apply Edge Detection\n";
    cout << "4. Apply Invert Colors Effect\n";
    cout << "5.Apply blur effect\n";
    cout << "6. Apply vintage effect\n";
    cout << "7. Exit\n";
    cout << "Enter your choice: ";
}

int main() {
    try {
        ifstream inputFile("input.txt");
        if (!inputFile.is_open()) {
            throw runtime_error("Error: Could not open input.txt");
        }

        string inputImage;
        getline(inputFile, inputImage);
        inputFile.close();

        int choice;
        do {
            displayMenu();
            cin >> choice;

            ImageProcessor* processor = nullptr;
            switch (choice) {
                case 1: processor = new GrayscaleConverter(); break;
                case 2: processor = new SepiaEffect(); break;
                case 3: processor = new EdgeDetection(); break;
                case 4: processor = new InvertColorsEffect(); break;
                case 5: processor = new ExtremeBlurEffect();break; 
                case 6: processor = new VintageEffect();break; 
                case 7: cout << "Exiting program...\n"; return 0;
                default: cout << "Invalid choice! Please try again.\n"; continue;
            }

            if (processor->loadImage(inputImage)) {
                processor->processImage();
            }

            delete processor;

        } while (choice != 7);
    } catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
