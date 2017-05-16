/**
 ******************************************************************************
 ** ファイル名 : app.cpp
 **
 ** 概要 : 環境光、反射光等測定用
 **
 ** 注記 : 必要最小限のクラスの読み込みで確認
 **        Bluetoothは念のために実装しておく（テスト的にも使えるかもなんで）
 ******************************************************************************
 **/

#include "ev3api.h"
#include "app.h"
#include "TouchSensor.h"
#include "ColorSensor.h"
#include "Clock.h"
#include "Motor.h"
#include "Monitor.h"
#include <stdlib.h>

using namespace ev3api;

#define DEBUG

#ifdef DEBUG
#define _debug(x) (x)
#else
#define _debug(x)
#endif

/* Bluetooth */
static int32_t   bt_cmd = 0;      /* Bluetoothコマンド 1:リモートスタート */
static FILE     *bt = NULL;      /* Bluetoothファイルハンドル */

/* 下記のマクロは個体/環境に合わせて変更する必要があります */
#define TAIL_ANGLE_STAND_UP  90  /* 完全停止時の角度[度] */
#define P_GAIN             2.5F  /* 完全停止用モータ制御比例係数 */
#define PWM_ABS_MAX          60  /* 完全停止用モータ制御PWM絶対最大値 */
//#define DEVICE_NAME     "ET0"  /* Bluetooth名 hrp2/target/ev3.h BLUETOOTH_LOCAL_NAMEで設定 */
//#define PASS_KEY        "1234" /* パスキー    hrp2/target/ev3.h BLUETOOTH_PIN_CODEで設定 */
#define CMD_START         '1'    /* リモートスタートコマンド */

/* 関数プロトタイプ宣言 */
static void tailControl(int32_t angle);
static void waitTouch(void);
static void waitNext(int32_t roopNumber);
static char* getColorName(colorid_t color);

/* オブジェクトへのポインタ定義 */
TouchSensor*    touchSensor;
ColorSensor*    colorSensor;
Motor*          tailMotor;
Monitor*        monitor;
Clock*          clock;

/* メインタスク */
void main_task(intptr_t unused)
{
    /* 各オブジェクトを生成・初期化する */
    touchSensor = new TouchSensor(PORT_1);
    colorSensor = new ColorSensor(PORT_3);
    tailMotor   = new Motor(PORT_A);
    monitor     = new Monitor();
    clock       = new Clock();

    uint8_t     ambient;
    int8_t      brightness;
    colorid_t   color_number;
    rgb_raw_t   raw_color_data;
    rgb_raw_t   *raw_color = &raw_color_data;
    char        buf[1000];
    ledcolor_t led_color[3] = {LED_GREEN,LED_ORANGE,LED_RED};

    /* LCD画面表示 */
    monitor->display("start monitoring-test");

    /* 尻尾モーターのリセット */
    tailMotor->reset();

    /* Open Bluetooth file */
    bt = ev3_serial_open_file(EV3_SERIAL_BT);
    assert(bt != NULL);

    /* Bluetooth通信タスクの起動 */
    act_tsk(BT_TASK);

    // 測定は３回行える
    for(int i=0; i<3; i++)
    {
        /* 初期化完了通知 */
        ev3_led_set_color(led_color[i]);

        /* n回目のタッチセンサー待機 */
        waitTouch();

        /* 光測定 + ディスプレイ表示 */
        /** 環境光の強さ **/
        ambient         = colorSensor->getAmbient();
        sprintf(buf,"Ambient is %-d",ambient);
        monitor->display(buf);

        /** 反射光の強さ **/
        brightness      = colorSensor->getBrightness();
        sprintf(buf,"Brightness is %-d",brightness);
        monitor->display(buf);

        /** 識別した色を取得する **/
        color_number    = colorSensor->getColorNumber();
        sprintf(buf,"ColorNumber is %-d",color_number);
        sprintf(buf,"ColorName is %s",getColorName(color_number));
        monitor->display(buf);

        /** RGB Raw値を測定する **/
        colorSensor->getRawColor(*raw_color);
        sprintf(buf,"RGB is %-d , %-d , %-d",raw_color->r,raw_color->g,raw_color->b);
        monitor->display(buf);

        waitNext(300);
    }

    /* 完了通知 */
    ev3_led_set_color(LED_OFF);
    monitor->display("end monitoring-test");

    /* 尻尾モーターのリセット */
    tailMotor->reset();

    ter_tsk(BT_TASK);
    fclose(bt);

    delete touchSensor;
    delete colorSensor;
    delete tailMotor;
    delete monitor;
    delete clock;

    ext_tsk();
}

//*****************************************************************************
// 関数名 : tailControl
// 引数 : angle (モータ目標角度[度])
// 返り値 : 無し
// 概要 : 走行体完全停止用モータの角度制御
//*****************************************************************************
static void tailControl(int32_t angle)
{
    float pwm = (float)(angle - tailMotor->getCount()) * P_GAIN; /* 比例制御 */
    /* PWM出力飽和処理 */
    if (pwm > PWM_ABS_MAX)
    {
        pwm = PWM_ABS_MAX;
    }
    else if (pwm < -PWM_ABS_MAX)
    {
        pwm = -PWM_ABS_MAX;
    }

    tailMotor->setPWM(pwm);
}

//*****************************************************************************
// 関数名 : waitTouch
// 引数 : なし
// 返り値 : なし
// 概要 : タッチセンサーが押されるまでループを繰り返す
//*****************************************************************************
static void waitTouch(void)
{
    while(1)
    {

        /* 完全停止用角度に制御 */
        tailControl(TAIL_ANGLE_STAND_UP);

        if (bt_cmd == 1)
        {
            break; /* リモートスタート */
        }

        if (touchSensor->isPressed())
        {
            break; /* タッチセンサが押された */
        }

        /* 10msec周期起動 */
        clock->sleep(10);
    }
}

//*****************************************************************************
// 関数名 : waitNext
// 引数 : なし
// 返り値 : なし
// 概要 : ループを抜けるまで繰り返す
//*****************************************************************************
static void waitNext(int32_t roopNumber)
{
    int32_t i = 1;
    while(i != roopNumber)
    {

        /* 完全停止用角度に制御 */
        tailControl(TAIL_ANGLE_STAND_UP);

        /* 10msec周期起動 */
        clock->sleep(10);

        i++;
    }
}

//*****************************************************************************
// 関数名 : getColorName
// 引数 : colorid_t
// 返り値 : char*　色名
// 概要 : 引数で渡されるカラーの番号から色名を英語で返す
//*****************************************************************************
static char* getColorName(colorid_t color)
{
    switch (color) {
        case COLOR_NONE:
            return (char*)"None";
            break;
        case COLOR_BLACK:
            return (char*)"Black";
            break;
        case COLOR_BLUE:
            return (char*)"Blue";
            break;
        case COLOR_GREEN:
            return (char*)"Green";
            break;
        case COLOR_YELLOW:
            return (char*)"Yellow";
            break;
        case COLOR_RED:
            return (char*)"Red";
            break;
        case COLOR_WHITE:
            return (char*)"White";
            break;
        case COLOR_BROWN:
            return (char*)"Brown";
            break;
        default:
            return (char*)"Unknown";
            break;
    }
}

//*****************************************************************************
// 関数名 : bt_task
// 引数 : unused
// 返り値 : なし
// 概要 : Bluetooth通信によるリモートスタート。 Tera Termなどのターミナルソフトから、
//       ASCIIコードで1を送信すると、リモートスタートする。
//*****************************************************************************
void bt_task(intptr_t unused)
{
    while(1)
    {
        uint8_t c = fgetc(bt); /* 受信 */
        switch(c)
        {
        case '1':
            bt_cmd = 1;
            break;
        default:
            break;
        }
        fputc(c, bt); /* エコーバック */
    }
}
