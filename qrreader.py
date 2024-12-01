import cv2
from pyzbar.pyzbar import decode
import numpy as np
import sys
import os
from PIL import Image, ImageTk
import tkinter as tk
from tkinter import messagebox
from pillow_heif import register_heif_opener

class QRCodeReader:
    def __init__(self, root, image_path):
        self.root = root
        self.root.title("QR Code Reader")
        
        # Get screen dimensions
        screen_width = root.winfo_screenwidth()
        screen_height = root.winfo_screenheight()
        
        # Set maximum dimensions (80% of screen size)
        self.max_width = int(screen_width * 0.8)
        self.max_height = int(screen_height * 0.8)
        
        # Image loading and processing
        self.original_image = self.load_image(image_path)
        if self.original_image is None:
            messagebox.showerror("Error", "Could not load image")
            self.root.quit()
            return
        
        # Resize image if needed
        self.scale_factor = 1.0
        self.resized_image = self.resize_image_to_fit(self.original_image)
            
        # Convert to PIL format for tkinter
        self.display_image = self.convert_to_display(self.resized_image)
        
        # Initialize variables for rectangle drawing
        self.start_x = None
        self.start_y = None
        self.rect_id = None
        self.selection = None
        
        # Create canvas with resized dimensions
        self.canvas = tk.Canvas(
            root, 
            width=self.display_image.width(), 
            height=self.display_image.height()
        )
        self.canvas.pack(side="top", fill="both", expand=True)
        
        # Display image on canvas
        self.canvas_image = self.canvas.create_image(
            0, 0, anchor="nw", image=self.display_image
        )
        
        # Bind mouse events
        self.canvas.bind("<ButtonPress-1>", self.on_press)
        self.canvas.bind("<B1-Motion>", self.on_drag)
        self.canvas.bind("<ButtonRelease-1>", self.on_release)
        
        # Create buttons
        button_frame = tk.Frame(root)
        button_frame.pack(side="bottom", fill="x", padx=5, pady=5)
        
        tk.Button(
            button_frame, 
            text="Decode QR", 
            command=self.decode_qr
        ).pack(side="left", padx=5)
        
        tk.Button(
            button_frame, 
            text="Clear Selection", 
            command=self.clear_selection
        ).pack(side="left", padx=5)
        
        tk.Button(
            button_frame, 
            text="Exit", 
            command=root.quit
        ).pack(side="right", padx=5)

        # Center window on screen
        self.center_window()

    def resize_image_to_fit(self, image):
        height, width = image.shape[:2]
        
        # Calculate scale factor to fit within max dimensions
        width_scale = self.max_width / width
        height_scale = self.max_height / height
        self.scale_factor = min(width_scale, height_scale)
        
        # Always resize using the scale factor
        new_width = int(width * self.scale_factor)
        new_height = int(height * self.scale_factor)
        resized = cv2.resize(image, (new_width, new_height), 
                            interpolation=cv2.INTER_AREA)
        return resized

    def center_window(self):
        # Update window to get actual dimensions
        self.root.update_idletasks()
        
        # Calculate position for center of screen
        window_width = self.root.winfo_width()
        window_height = self.root.winfo_height()
        position_x = (self.root.winfo_screenwidth() // 2) - (window_width // 2)
        position_y = (self.root.winfo_screenheight() // 2) - (window_height // 2)
        
        # Set window position
        self.root.geometry(f"+{position_x}+{position_y}")

    def load_image(self, image_path):
        try:
            file_ext = os.path.splitext(image_path)[1].lower()
            
            if file_ext in ['.heic', '.heif']:
                register_heif_opener()
                pil_image = Image.open(image_path)
                rgb_image = pil_image.convert('RGB')
                opencv_image = np.array(rgb_image)
                return cv2.cvtColor(opencv_image, cv2.COLOR_RGB2BGR)
            else:
                return cv2.imread(image_path)
        except Exception as e:
            messagebox.showerror("Error", f"Error loading image: {str(e)}")
            return None

    def convert_to_display(self, opencv_image):
        # Convert OpenCV image to PIL format
        color_converted = cv2.cvtColor(opencv_image, cv2.COLOR_BGR2RGB)
        pil_image = Image.fromarray(color_converted)
        return ImageTk.PhotoImage(pil_image)

    def on_press(self, event):
        self.start_x = event.x
        self.start_y = event.y
        
        if self.rect_id:
            self.canvas.delete(self.rect_id)
        self.rect_id = self.canvas.create_rectangle(
            self.start_x, self.start_y, 
            self.start_x, self.start_y, 
            outline='red', width=2
        )

    def on_drag(self, event):
        if self.rect_id:
            self.canvas.coords(
                self.rect_id, 
                self.start_x, self.start_y, 
                event.x, event.y
            )

    def on_release(self, event):
        x1 = min(self.start_x, event.x)
        y1 = min(self.start_y, event.y)
        x2 = max(self.start_x, event.x)
        y2 = max(self.start_y, event.y)
        self.selection = (x1, y1, x2, y2)

    def clear_selection(self):
        if self.rect_id:
            self.canvas.delete(self.rect_id)
            self.rect_id = None
            self.selection = None

    def decode_qr(self):
        if not self.selection:
            messagebox.showwarning("Warning", "Please select QR code region first")
            return
        
        try:
            # Convert selection coordinates back to original image scale
            x1, y1, x2, y2 = self.selection
            # Use inverse of scale_factor to get original coordinates
            orig_x1 = int(x1 / self.scale_factor)
            orig_y1 = int(y1 / self.scale_factor)
            orig_x2 = int(x2 / self.scale_factor)
            orig_y2 = int(y2 / self.scale_factor)
            
            # Ensure coordinates are within image bounds
            height, width = self.original_image.shape[:2]
            orig_x1 = max(0, min(orig_x1, width))
            orig_x2 = max(0, min(orig_x2, width))
            orig_y1 = max(0, min(orig_y1, height))
            orig_y2 = max(0, min(orig_y2, height))
            
            # Debug print
            print(f"Display image size: {self.display_image.width()}x{self.display_image.height()}")
            print(f"Original image size: {width}x{height}")
            print(f"Scale factor: {self.scale_factor}")
            print(f"Selection on display: ({x1},{y1}) to ({x2},{y2})")
            print(f"Selection on original: ({orig_x1},{orig_y1}) to ({orig_x2},{orig_y2})")
            
            # Crop original image
            cropped = self.original_image[orig_y1:orig_y2, orig_x1:orig_x2]
            
            # Debug: Show cropped region
            cv2.imshow('Cropped Region', cropped)
            
            # Convert to grayscale
            gray = cv2.cvtColor(cropped, cv2.COLOR_BGR2GRAY)
            
            # Debug: Show grayscale image
            cv2.imshow('Grayscale', gray)
            
            # Try different thresholding methods
            _, binary = cv2.threshold(gray, 128, 255, cv2.THRESH_BINARY)
            cv2.imshow('Binary', binary)
            
            # Debug: Print decode attempt
            decoded_objects = decode(gray)
            print(f"Number of QR codes found: {len(decoded_objects)}")
            
            if not decoded_objects:
                # Try decoding with binary image
                decoded_objects = decode(binary)
                print(f"Number of QR codes found after binary threshold: {len(decoded_objects)}")
                
            if not decoded_objects:
                messagebox.showinfo("Result", "No QR code found in selected region")
                return
            
            # Show results
            for obj in decoded_objects:
                result = f"Type: {obj.type}\nData: {obj.data.decode('utf-8')}"
                messagebox.showinfo("QR Code Content", result)
                
        except Exception as e:
            print(f"Debug - Exception details: {str(e)}")
            messagebox.showerror("Error", f"Error decoding QR code: {str(e)}")
        
        # Wait for key press before closing debug windows
        cv2.waitKey(0)
        cv2.destroyAllWindows()

def print_usage():
    print("Usage: python qrreader.py <image_file_path>")
    print("Supported formats: JPG, PNG, HEIC")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print_usage()
        sys.exit(1)
    
    image_path = sys.argv[1]
    if not os.path.exists(image_path):
        print(f"Error: Image file not found: {image_path}")
        sys.exit(1)
        
    root = tk.Tk()
    app = QRCodeReader(root, image_path)
    root.mainloop()
