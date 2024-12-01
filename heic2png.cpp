#include <iostream>
#include <string>
#include <vector>
#include <libheif/heif.h>
#include <png.h>

bool convert_heic_to_png(const std::string& input_path) {
    // Create HEIF context
    heif_context* ctx = heif_context_alloc();
    if (!ctx) {
        std::cerr << "Could not create HEIF context\n";
        return false;
    }

    // Read HEIC file
    heif_error error = heif_context_read_from_file(ctx, input_path.c_str(), nullptr);
    if (error.code != heif_error_Ok) {
        std::cerr << "Could not read HEIC file: " << error.message << "\n";
        heif_context_free(ctx);
        return false;
    }

    // Get primary image handle
    heif_image_handle* handle;
    error = heif_context_get_primary_image_handle(ctx, &handle);
    if (error.code != heif_error_Ok) {
        std::cerr << "Could not get image handle: " << error.message << "\n";
        heif_context_free(ctx);
        return false;
    }

    // Decode the image
    heif_image* img;
    error = heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGB, nullptr);
    if (error.code != heif_error_Ok) {
        std::cerr << "Could not decode image: " << error.message << "\n";
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return false;
    }

    // Get image dimensions and data
    int width = heif_image_get_width(img, heif_channel_interleaved);
    int height = heif_image_get_height(img, heif_channel_interleaved);
    int stride;
    const uint8_t* data = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);

    // Create output filename (replace .heic with .png)
    std::string output_path = input_path;
    size_t ext_pos = output_path.rfind(".heic");
    if (ext_pos != std::string::npos) {
        output_path.replace(ext_pos, 5, ".png");
    }

    // Create PNG file
    FILE* fp = fopen(output_path.c_str(), "wb");
    if (!fp) {
        std::cerr << "Could not create output PNG file\n";
        heif_image_release(img);
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return false;
    }

    // Initialize PNG structures
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        std::cerr << "Could not create PNG write structure\n";
        fclose(fp);
        heif_image_release(img);
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return false;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        std::cerr << "Could not create PNG info structure\n";
        png_destroy_write_struct(&png, nullptr);
        fclose(fp);
        heif_image_release(img);
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return false;
    }

    // Set up error handling
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        heif_image_release(img);
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return false;
    }

    // Initialize PNG I/O
    png_init_io(png, fp);

    // Write PNG header
    png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    // Write image data
    std::vector<png_bytep> row_pointers(height);
    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_bytep)(data + y * stride);
    }
    png_write_image(png, row_pointers.data());
    png_write_end(png, nullptr);

    // Clean up
    png_destroy_write_struct(&png, &info);
    fclose(fp);
    heif_image_release(img);
    heif_image_handle_release(handle);
    heif_context_free(ctx);

    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.heic>\n";
        return 1;
    }

    if (convert_heic_to_png(argv[1])) {
        std::cout << "Conversion successful\n";
        return 0;
    } else {
        std::cerr << "Conversion failed\n";
        return 1;
    }
}
