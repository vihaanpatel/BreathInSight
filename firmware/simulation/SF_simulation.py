import pygame
import random
import time
import math

# Initialize Pygame
pygame.init()

# Constants for display
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600
OLED_WIDTH = 128  # From original code
OLED_HEIGHT = 64  # From original code
OLED_SCALE = 3    # Scale up the OLED display for visibility

# Colors
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
BLUE = (55, 55, 55)

# Create main window
screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
pygame.display.set_caption("Sensor Simulation")

# Create scaled OLED surface
oled = pygame.Surface((OLED_WIDTH * OLED_SCALE, OLED_HEIGHT * OLED_SCALE))

class SensorSimulation:
    def __init__(self):
        self.voc_ppb = 0
        self.voc_ppm = 0
        self.fev1_percent = 80  # Normal baseline FEV1 (changed from 65)
        self.breathing_phase = 0
        self.font = pygame.font.Font(None, 24)
        
    def simulate_breath(self):
        # Simulate normal breathing pattern using sine wave
        self.breathing_phase += 0.08  # Normal breathing rate (increased from 0.05)
        breath_intensity = math.sin(self.breathing_phase)  # Normal breath intensity (removed 0.7 reduction)
        
        # Simulate normal VOC readings
        base_voc = 0
        variation = random.uniform(-300, 800)  # More balanced variation (changed from -500, 1200)
        self.voc_ppb = base_voc + (breath_intensity * 1200) + variation  # Reduced intensity
        self.voc_ppm = self.voc_ppb / 1000
        self.voc_ppm = max(0, self.voc_ppm)  # Ensure VOC ppm is not negative
        
        # Simulate normal FEV1 changes
        self.fev1_percent = 80 + (breath_intensity * 5)  # Higher baseline and more variation
        
    def get_risk_level(self, voc_ppm, voc_ppb, fev1):
        # Simplified risk assessment based on original code
        copd_risk = "Low"
        if voc_ppm >= 0.24 and voc_ppm <= 0.76:  # Based on COPD_VOC_THRESHOLD ± 0.26
            copd_risk = "High"
            
        asthma_risk = "Low"
        if voc_ppb > 2335:  # Based on ASTHMA_VOC_THRESHOLD
            asthma_risk = "High"
            
        cancer_risk = "Low"
        if 0.4 <= voc_ppm <= 0.7:  # Based on LUNG_CANCER_VOC thresholds
            cancer_risk = "High"
            
        return copd_risk, asthma_risk, cancer_risk
    
    def draw_oled(self):
        oled.fill(BLACK)
        
        # Draw readings similar to original display code
        y_spacing = 16 * OLED_SCALE
        
        # VOC Reading
        text = self.font.render(f"VOC (ppb): {self.voc_ppb:.2f}", True, WHITE)
        oled.blit(text, (0, 0))
        
        # FEV1 Reading
        text = self.font.render(f"FEV1: {self.fev1_percent:.1f}%", True, WHITE)
        oled.blit(text, (OLED_WIDTH * OLED_SCALE - text.get_width(), 0))
        
        # Risk Levels
        copd_risk, asthma_risk, cancer_risk = self.get_risk_level(
            self.voc_ppm, self.voc_ppb, self.fev1_percent
        )
        
        text = self.font.render(f"COPD Risk: {copd_risk}", True, WHITE)
        oled.blit(text, (0, y_spacing))
        
        text = self.font.render(f"Asthma Risk: {asthma_risk}", True, WHITE)
        oled.blit(text, (0, y_spacing * 2))
        
        text = self.font.render(f"Cancer Risk: {cancer_risk}", True, WHITE)
        oled.blit(text, (0, y_spacing * 3))

def main():
    simulation = SensorSimulation()
    running = True
    clock = pygame.time.Clock()
    
    while running:
        clock.tick(1)
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                
        # Clear screen
        screen.fill(BLUE)
        
        # Update simulation
        simulation.simulate_breath()
        
        # Draw OLED display
        simulation.draw_oled()
        
        # Draw the OLED surface to main screen
        oled_pos = ((SCREEN_WIDTH - OLED_WIDTH * OLED_SCALE) // 2,
                    (SCREEN_HEIGHT - OLED_HEIGHT * OLED_SCALE) // 2)
        screen.blit(oled, oled_pos)
        
        pygame.display.flip()
        clock.tick(15)  # Slower tick rate (was 30)
        
    pygame.quit()

if __name__ == "__main__":
    main()
