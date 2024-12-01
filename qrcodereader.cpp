#include <opencv2/opencv.hpp>
#include <libheif/heif.h>
#include <iostream>
#include <string>
#include <algorithm>

// Helper function to convert HEIC to cv::Mat
cv::Mat heicToCvMat(const std::string& imagePath) {
    heif_context* ctx = heif_context_alloc();
    heif_error err;
    
    // Read HEIC file
    err = heif_context_read_from_file(ctx, imagePath.c_str(), nullptr);
    if (err.code != heif_error_Ok) {
        heif_context_free(ctx);
        throw std::runtime_error("Could not read HEIC file: " + std::string(err.message));
    }

    // Get handle to primary image
    heif_image_handle* handle;
    err = heif_context_get_primary_image_handle(ctx, &handle);
    if (err.code != heif_error_Ok) {
        heif_context_free(ctx);
        throw std::runtime_error("Could not get image handle: " + std::string(err.message));
    }

    // Decode the image
    heif_image* img;
    err = heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGB, nullptr);
    if (err.code != heif_error_Ok) {
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        throw std::runtime_error("Could not decode image: " + std::string(err.message));
    }

    // Get the image data
    int stride;
    const uint8_t* data = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);
    int width = heif_image_get_width(img, heif_channel_interleaved);
    int height = heif_image_get_height(img, heif_channel_interleaved);

    // Create OpenCV Mat and copy data properly considering stride
    cv::Mat result(height, width, CV_8UC3);
    for (int y = 0; y < height; y++) {
        std::memcpy(result.ptr(y), data + y * stride, width * 3);
    }

    // Clean up
    heif_image_release(img);
    heif_image_handle_release(handle);
    heif_context_free(ctx);

    // Convert from RGB to BGR (OpenCV format)
    cv::cvtColor(result, result, cv::COLOR_RGB2BGR);
    return result;
}

bool isHeicFile(const std::string& filename) {
    std::string ext = filename.substr(filename.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return (ext == "heic" || ext == "heif");
}

void decodeQRCode(const std::string& imagePath) {
    try {
        // Read the image using OpenCV or HEIC decoder
        cv::Mat image;
        if (isHeicFile(imagePath)) {
            image = heicToCvMat(imagePath);
        } else {
            image = cv::imread(imagePath);
        }
        
        if (image.empty()) {
            throw std::runtime_error("Could not read the image file");
        }

        // Create QR code detector object
        cv::QRCodeDetector qrDecoder;
        
        // Variables to store the detection result
        std::string data;
        std::vector<cv::Point> bbox;
        cv::Mat straight_qrcode;
        
        // Detect and decode QR code
        data = qrDecoder.detectAndDecode(image, bbox, straight_qrcode);
        
        if (!data.empty()) {
            std::cout << "Decoded Data: " << data << std::endl;
        } else {
            std::cout << "No QR code found in the image" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <path_to_qr_code_image>" << std::endl;
        return 1;
    }
    
    std::string imagePath = argv[1];
    decodeQRCode(imagePath);
    
    return 0;
}
