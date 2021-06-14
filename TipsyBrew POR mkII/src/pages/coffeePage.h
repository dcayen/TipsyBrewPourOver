#ifndef COFFEE_PAGE
#define COFFEE_PAGE

#include <Arduino.h>

enum CoffeePage {
    PREPARE,
    WEIGH,
    BREWING
};

CoffeePage currCoffeePage = PREPARE;

void initCoffee() {
    switch (currCoffeePage) {
        case PREPARE:
            tft.setTextColor(MOSTLY_WHITE, BKGD);
            tft.setTextDatum(TL_DATUM);
            tft.drawString("PREPARE", 55, 65, 4);
            tft.drawString("1. Put Carafe in TipsyBrew", 55, 65+5+tft.fontHeight(4), 2);
            tft.drawString("2. Put EMPTY Cone in TipsyBrew", 55, 65+(2*5)+tft.fontHeight(2)+tft.fontHeight(4), 2);
            tft.drawString("3. Put Filter in Cone", 55, 65+(3*5)+(2*tft.fontHeight(2))+tft.fontHeight(4), 2);
            
            drawLittleButton(LEFT_OFF, TOP, rain, TFT_BLUE, MOSTLY_WHITE);
            drawLittleButton(LEFT_OFF, BOTTOM, home, TFT_BLACK, MOSTLY_WHITE);
            drawLittleButton(RIGHT_OFF, TOP, cold, TFT_BLACK, GREY_GRAY);
            drawLittleButton(RIGHT_OFF, BOTTOM, arrowRight, MOSTLY_WHITE, TB_ORANGE);
            break;
        case WEIGH:
            tft.setTextColor(MOSTLY_WHITE, BKGD);
            break;
        case BREWING:
            tft.setTextColor(MOSTLY_WHITE, BKGD);
            break;
    }
}

void coffeeTouch(int x, int y) {
    switch (currCoffeePage) {
        case PREPARE:
            if(x < 55 && y > 25 && y < tft.height()/2) {
                // Top Left (Rinse Filter)
                // TODO: Make a filter rinse function you dim witted twat
            } else if(x < 55 && y > tft.height()/2 && y < tft.height()-25) {
                // Bottom Left (Home)
                appState.currentScreen = APP_HOME;
                appState.screenRefresh = true;
            } else if (x > 265 && y > 25 && y < tft.height()/2) {
                // Top Right (Iced Coffee)
                // TODO: This is a special case of the normal brew, implement it AFTER a normal brew is done
            } else if (x > 265 && y > tft.height()/2 && y < tft.height()-25) {
                // Bottom Right (Continue with Brew)
                currCoffeePage = WEIGH;
                appState.screenRefresh = true;
            };
            break;
        case WEIGH:
            // This screen doesn't exist yet, so pay no attention to my copy pasta here
            if(x < 55 && y > 25 && y < tft.height()/2) {
                // Top Left (Rinse Filter)
                // TODO: Make a filter rinse function you dim witted twat
            } else if(x < 55 && y > tft.height()/2 && y < tft.height()-25) {
                // Bottom Left (Home)
                currCoffeePage = PREPARE;
                appState.screenRefresh = true;
            } else if (x > 265 && y > 25 && y < tft.height()/2) {
                // Top Right (Iced Coffee)
                // TODO: This is a special case of the normal brew, implement it AFTER a normal brew is done
            } else if (x > 265 && y > tft.height()/2 && y < tft.height()-25) {
                // Bottom Right (Continue with Brew)
                currCoffeePage = BREWING;
                appState.screenRefresh = true;
            };
            break;
    }
}

#endif