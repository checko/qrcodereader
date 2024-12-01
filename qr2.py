import cv2
import sys
import numpy as np
from pillow_heif import register_heif_opener
from PIL import Image

def decode_qr_code(image_path):
    try:
        # Register HEIF opener for HEIC support
        register_heif_opener()
        
        # Handle HEIC files differently
        if image_path.lower().endswith(('.heic', '.heif')):
            # Open HEIC image with Pillow
            pil_image = Image.open(image_path)
            # Convert PIL image to numpy array for OpenCV
            image = cv2.cvtColor(np.array(pil_image), cv2.COLOR_RGB2BGR)
        else:
            # Regular image handling
            image = cv2.imread(image_path)
        
        if image is None:
            raise Exception("Could not read the image file")

        # Create QR code detector object
        qr_detector = cv2.QRCodeDetector()
        
        # Detect and decode QR code
        data, bbox, straight_qrcode = qr_detector.detectAndDecode(image)
        
        if data:
            print(f"Decoded Data: {data}")
        else:
            print("No QR code found in the image")
            
    except Exception as e:
        print(f"Error: {str(e)}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python qr2.py <path_to_qr_code_image>")
        sys.exit(1)
        
    image_path = sys.argv[1]
    decode_qr_code(image_path)
