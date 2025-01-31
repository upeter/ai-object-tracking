#include <Arduino.h>
#include <ESP32Servo.h>

#define SERVO1_PIN 18  // X-axis servo
#define SERVO2_PIN 27  // Y-axis servo

// Structure to hold rectangle data
struct Rectangle {
    float x;      // top-left x coordinate
    float y;      // top-left y coordinate
    float width;
    float height;
};

// Shared data structure with protection
struct SharedData {
    float targetX;
    float targetY;
    SemaphoreHandle_t mutex;
} shared;

// Task handles
TaskHandle_t servoTaskHandle = NULL;
TaskHandle_t inputTaskHandle = NULL;
Servo servoX;  // Handles X-axis movement
Servo servoY;  // Handles Y-axis movement

void inputTask(void *parameter) {
    while (true) {
        // Generate random rectangle coordinates
        float x = random(0, 120);      // Random x between 0-120 to allow for width
        float y = random(0, 120);      // Random y between 0-120 to allow for height
        float width = random(20, 60);  // Random width between 20-60
        float height = random(20, 60); // Random height between 20-60
        
        // Calculate center coordinates
        float centerX = x + (width / 2);
        float centerY = y + (height / 2);
        
        // Update shared data with protection
        xSemaphoreTake(shared.mutex, portMAX_DELAY);
        shared.targetX = centerX;
        shared.targetY = centerY;
        xSemaphoreGive(shared.mutex);
        
        Serial.printf("New random rect: x=%.2f, y=%.2f, w=%.2f, h=%.2f\n", x, y, width, height);
        Serial.printf("New target position: X=%.2f, Y=%.2f\n", centerX, centerY);
        
        // Wait 10 seconds before generating new coordinates
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void servoTask(void *parameter) {
    float currentX = 90;  // Start at center position
    float currentY = 90;
    
    while (true) {
        float targetX, targetY;
        
        // Get target coordinates with protection
        xSemaphoreTake(shared.mutex, portMAX_DELAY);
        targetX = shared.targetX;
        targetY = shared.targetY;
        xSemaphoreGive(shared.mutex);
        
        // Move X servo first
        if (abs(currentX - targetX) > 0.5) {
            if (currentX < targetX) {
                currentX += 2;  // Increased step size
            } else {
                currentX -= 2;  // Increased step size
            }
            servoX.write(currentX);
            vTaskDelay(pdMS_TO_TICKS(25));  // Reduced delay
        }
        
        // Only start Y movement after X is done
        if (abs(currentX - targetX) <= 0.5) {
            if (abs(currentY - targetY) > 0.5) {
                if (currentY < targetY) {
                    currentY += 2;  // Increased step size
                } else {
                    currentY -= 2;  // Increased step size
                }
                servoY.write(currentY);
                vTaskDelay(pdMS_TO_TICKS(25));  // Reduced delay
            }
        }
        
        // Small delay before next iteration
        vTaskDelay(pdMS_TO_TICKS(10));  // Reduced delay
    }
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    
    // Initialize random number generator
    randomSeed(analogRead(0));
    
    // Initialize mutex for shared data
    shared.mutex = xSemaphoreCreateMutex();
    shared.targetX = 90;  // Start at center position
    shared.targetY = 90;
    
    servoX.attach(SERVO1_PIN);
    servoY.attach(SERVO2_PIN);
    
    // Wait for serial connection
    while (!Serial) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    Serial.println("ESP32 Ready! Generating random rectangles every 10 seconds");

    // Create tasks
    xTaskCreate(
        inputTask,
        "Input Task",
        2048,
        NULL,
        2,  // Higher priority for input handling
        &inputTaskHandle
    );

    xTaskCreate(
        servoTask,
        "Servo Task",
        1024,
        NULL,
        1,
        &servoTaskHandle
    );
}

void loop() {
    // Suspend the loop task forever
    vTaskSuspend(NULL);
}