#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/objdetect.hpp>
#include <libheif/heif.h>
#include <iostream>

cv::Mat readHEIC(const std::string& filename) {
    heif_context* ctx = heif_context_alloc();
    heif_error err = heif_context_read_from_file(ctx, filename.c_str(), nullptr);
    if (err.code != heif_error_Ok) {
        std::cerr << "Error reading HEIC file: " << err.message << std::endl;
        heif_context_free(ctx);
        return cv::Mat();
    }

    heif_image_handle* handle;
    err = heif_context_get_primary_image_handle(ctx, &handle);
    if (err.code != heif_error_Ok) {
        std::cerr << "Error getting primary image handle: " << err.message << std::endl;
        heif_context_free(ctx);
        return cv::Mat();
    }

    heif_image* img;
    err = heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGB, nullptr);
    if (err.code != heif_error_Ok) {
        std::cerr << "Error decoding HEIC image: " << err.message << std::endl;
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return cv::Mat();
    }

    int stride;
    const uint8_t* data = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);
    if (!data) {
        std::cerr << "Error accessing image data!" << std::endl;
        heif_image_release(img);
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return cv::Mat();
    }

    int width = heif_image_get_width(img, heif_channel_interleaved);
    int height = heif_image_get_height(img, heif_channel_interleaved);

    cv::Mat image(height, width, CV_8UC3, (void*)data, stride);

    heif_image_release(img);
    heif_image_handle_release(handle);
    heif_context_free(ctx);

    return image.clone(); // Clone to ensure data is copied
}

int main() {
    // Load the HEIC image
    cv::Mat image = readHEIC("./qrcode.heic");
    if (image.empty()) {
        std::cerr << "Could not open or find the image!" << std::endl;
        return -1;
    }

    // Initialize the QRCode detector
    cv::QRCodeDetector qrDecoder = cv::QRCodeDetector();

    // Detect and decode the QR code
    std::string data = qrDecoder.detectAndDecode(image);

    // Check if a QR code was detected
    if (data.empty()) {
        std::cout << "QR Code not detected" << std::endl;
    } else {
        std::cout << "Decoded data: " << data << std::endl;
    }

    return 0;
}
