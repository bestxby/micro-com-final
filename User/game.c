#include "game.h"
#include "lcd.h"
#include <stdio.h>

#define STATE_START    0
#define STATE_PLAY     1
#define STATE_GAMEOVER 2

/* 内部游戏变量 */
static uint8_t game_state = STATE_START;
static float bird_y = 160.0f;
static float bird_vy = 0.0f;
static const int bird_x = 120;
static float old_bird_y = 160.0f;

static float pipe_x = 440.0f;
static float old_pipe_x = 440.0f;
static int gap_y = 160;
static const int gap_h = 75;  /* 通道高度 */
static const int pipe_w = 32;  /* 管道宽度 */

static int score = 0;
static int last_drawn_score = -1;
static uint8_t passed_pipe = 0;
static uint32_t game_ticks = 0;

/**
  * @brief  初始化飞鸟小游戏状态与参数。
  * @retval 无
  */
void Game_Init(void)
{
    game_state = STATE_START;
    bird_y = 160.0f;
    bird_vy = 0.0f;
    old_bird_y = 160.0f;
    pipe_x = 440.0f;
    old_pipe_x = 440.0f;
    gap_y = 160;
    score = 0;
    last_drawn_score = -1;
    passed_pipe = 0;
    game_ticks = 0;
}

/**
  * @brief  每 30ms 周期执行的游戏逻辑物理计算和状态机流转。
  * @param  key_pressed: 检测到的按键输入代码 (2 代表 KEY2)。
  * @retval 无
  */
void Game_Update(uint8_t key_pressed)
{
    if (game_state == STATE_START) {
        if (key_pressed == 2) { /* KEY2 按下，游戏正式开始 */
            game_state = STATE_PLAY;
            bird_vy = -4.5f; /* 初始拍击向上的速度 */
            Game_Draw(1); /* 立即擦除文本并重绘背景边框 */
        }
    } 
    else if (game_state == STATE_PLAY) {
        game_ticks++;

        /* 物理重力与拍击模拟 */
        if (key_pressed == 2) {
            bird_vy = -4.5f; /* 拍翅膀往上飞 */
        } else {
            bird_vy += 0.4f; /* 模拟重力下坠 */
            if (bird_vy > 8.0f) bird_vy = 8.0f; /* 限制最大下落速度 */
        }

        old_bird_y = bird_y;
        bird_y += bird_vy;

        /* 地面与天花板边缘碰撞检测 (游戏面板 Y: 56..286，小鸟高度 12px) */
        if (bird_y <= 57.0f) {
            bird_y = 57.0f;
            game_state = STATE_GAMEOVER;
        }
        if (bird_y + 11 >= 285.0f) {
            bird_y = 285.0f - 11;
            game_state = STATE_GAMEOVER;
        }

        /* 移动管道 */
        old_pipe_x = pipe_x;
        pipe_x -= 4.0f; /* 每帧移动 4 像素 */

        /* 管道出界后复位到最右侧，并生成新的随机 gap 位置 */
        if (pipe_x < 21 - pipe_w) {
            /* 在复位前，需要用黑色彻底抹除旧管道的最后残余 */
            int old_x = (int)old_pipe_x;
            int top_y2 = gap_y - gap_h / 2;
            int bot_y1 = gap_y + gap_h / 2;
            
            if (old_x < 460) {
                int erase_w = 460 - old_x;
                if (erase_w > pipe_w) erase_w = pipe_w;
                LCD_FillRect(old_x, 57, erase_w, top_y2 - 57, theme_bg);
                LCD_FillRect(old_x, bot_y1, erase_w, 285 - bot_y1, theme_bg);
            }

            pipe_x = 440.0f;
            old_pipe_x = 440.0f;

            /* 使用简易随机算法确定下一次通道的 Y 轴中心位置 */
            gap_y = 110 + (game_ticks % 100);
            passed_pipe = 0;
        }

        /* 飞鸟与管道碰撞检测 */
        int bx1 = bird_x;
        int bx2 = bird_x + 11;
        int by1 = (int)bird_y;
        int by2 = (int)bird_y + 11;

        int px1 = (int)pipe_x;
        int px2 = (int)pipe_x + pipe_w - 1;
        int top_limit = gap_y - gap_h / 2;
        int bot_limit = gap_y + gap_h / 2;

        /* 横向重合判断 */
        if (bx2 >= px1 && bx1 <= px2) {
            /* 纵向撞击上管道或下管道判断 */
            if (by1 < top_limit || by2 > bot_limit) {
                game_state = STATE_GAMEOVER;
            }
        }

        /* 得分计算：越过管道右边缘 */
        if (!passed_pipe && bx1 > px2) {
            score++;
            passed_pipe = 1;
        }
    } 
    else if (game_state == STATE_GAMEOVER) {
        if (key_pressed == 2) {
            Game_Init(); /* 重启并返回准备状态 */
            Game_Draw(1); /* 强制完整刷新一次 */
        }
    }
}

/**
  * @brief  增量式画面重绘函数，仅抹除和重绘像素变动区域，实现 30+ FPS 高频渲染。
  * @param  force_refresh: 为 1 时强制绘制静态游戏面板边框与启动背景。
  * @retval 无
  */
void Game_Draw(uint8_t force_refresh)
{
    char text_buf[32];
    
    if (force_refresh) {
        /* 绘制游戏大底座卡片框 (X: 20..460, Y: 56..286) */
        LCD_FillRect(20, 56, 441, 231, theme_bg);
        LCD_DrawRect(20, 56, 441, 231, theme_border);
        LCD_DrawRect(19, 55, 443, 233, theme_border); /* 双线高级边框 */
        
        // 绘制外框的科技感青色折角
        // 左上
        LCD_DrawLine(19, 55, 29, 55, theme_accent);
        LCD_DrawLine(19, 55, 19, 65, theme_accent);
        // 右上
        LCD_DrawLine(452, 55, 462, 55, theme_accent);
        LCD_DrawLine(462, 55, 462, 65, theme_accent);
        // 左下
        LCD_DrawLine(19, 278, 19, 288, theme_accent);
        LCD_DrawLine(19, 288, 29, 288, theme_accent);
        // 右下
        LCD_DrawLine(452, 288, 462, 288, theme_accent);
        LCD_DrawLine(462, 278, 462, 288, theme_accent);
        
        if (game_state == STATE_START) {
            LCD_FillRect(376, 26, 100, 19, theme_bg);
            LCD_ShowString(188, 120, "FLAPPY BIRD", theme_yellow, theme_bg);
            LCD_ShowString(152, 165, "Press UP to FLAP", theme_text, theme_bg);
            LCD_ShowString(152, 195, "Press L/R to EXIT", theme_text_muted, theme_bg);
        }
        last_drawn_score = -1;
    }
    
    if (game_state == STATE_PLAY) {
        /* 1. 增量渲染副标题的得分看板 (仅当得分改变时才重绘以防闪烁) */
        if (score != last_drawn_score) {
            sprintf(text_buf, "SCORE: %2d", score);
            LCD_ShowString(376, 28, text_buf, theme_accent, theme_bg);
            last_drawn_score = score;
        }

        /* 2. 增量渲染小鸟 */
        int old_by = (int)old_bird_y;
        int new_by = (int)bird_y;
        
        if (old_by != new_by || force_refresh) {
            if (!force_refresh) {
                /* 背景色抹除旧小鸟位置 */
                LCD_FillRect(bird_x, old_by, 12, 12, theme_bg);
            }
            /* 绘制新小鸟 (主题黄色) */
            LCD_FillRect(bird_x, new_by, 12, 12, theme_yellow);
        }

        /* 3. 增量渲染管道 (仅绘制管道运动的 4px 左右前/后边缘) */
        int old_px = (int)old_pipe_x;
        int new_px = (int)pipe_x;
        int top_limit = gap_y - gap_h / 2;
        int bot_limit = gap_y + gap_h / 2;

        if (old_px != new_px || force_refresh) {
            if (force_refresh) {
                /* 全屏重绘管道 */
                int px1 = new_px;
                int px2 = new_px + pipe_w - 1;
                if (px1 < 21) px1 = 21;
                if (px2 > 459) px2 = 459;
                if (px1 <= px2) {
                    LCD_FillRect(px1, 57, px2 - px1 + 1, top_limit - 57, theme_green);
                    LCD_FillRect(px1, bot_limit, px2 - px1 + 1, 285 - bot_limit, theme_green);
                }
            } else {
                int dx = old_px - new_px; /* 正常每帧向左偏移 4 像素 */
                if (dx > 0) {
                    /* (A) 用背景色抹除管道向左移动后露出的旧右侧边缘 */
                    int erase_x = new_px + pipe_w;
                    int erase_w = dx;
                    if (erase_x < 21) {
                        erase_w -= (21 - erase_x);
                        erase_x = 21;
                    }
                    if (erase_x + erase_w - 1 > 459) {
                        erase_w = 460 - erase_x;
                    }
                    if (erase_w > 0) {
                        LCD_FillRect(erase_x, 57, erase_w, top_limit - 57, theme_bg);
                        LCD_FillRect(erase_x, bot_limit, erase_w, 285 - bot_limit, theme_bg);
                    }

                    /* (B) 绘制管道向左移出的新左侧边缘 (填充主题绿) */
                    int draw_x = new_px;
                    int draw_w = dx;
                    if (draw_x < 21) {
                        draw_w -= (21 - draw_x);
                        draw_x = 21;
                    }
                    if (draw_x + draw_w - 1 > 459) {
                        draw_w = 460 - draw_x;
                    }
                    if (draw_w > 0) {
                        LCD_FillRect(draw_x, 57, draw_w, top_limit - 57, theme_green);
                        LCD_FillRect(draw_x, bot_limit, draw_w, 285 - bot_limit, theme_green);
                    }
                }
            }
        }
    } 
    else if (game_state == STATE_GAMEOVER) {
        /* 游戏结束界面 */
        LCD_ShowString(188, 120, "GAME OVER", theme_red, theme_bg);
        sprintf(text_buf, "Final Score: %2d", score);
        LCD_ShowString(164, 155, text_buf, theme_yellow, theme_bg);
        LCD_ShowString(132, 190, "Press UP to Restart", theme_text, theme_bg);
    }
}
