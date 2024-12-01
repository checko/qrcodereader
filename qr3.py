import cv2

# Load the image
image = cv2.imread('qrcode.png')

# Initialize the QRCode detector
detector = cv2.QRCodeDetector()

# Detect and decode the QR code
data, bbox, _ = detector.detectAndDecode(image)

# Print the decoded data
print(f'Decoded data: {data}')
