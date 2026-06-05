`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
module S_8254(
    input clk0,			 // Counter0输入时钟
    input gate0,		 // Counter0使能    
    output reg out0,	 // Counter0输出波形    
    input CS_N,			 // 片选
    input [1:0] a,		 // 地址线
    input [7:0] id,		 // 输入数据 
    output reg [7:0] od, // 输出数据
    input IOR_N,		 // 读信号
    input IOW_N			 // 写信号
    );
    
    //计时器内部寄存器
    reg [15:0] CR; //初始值寄存器：只用低8位
    reg [15:0] OL; //输出锁存器：只用低8位
    reg [7:0]  SR; //状态寄存器
    reg [7:0]  SL; //状态锁存器
    reg [15:0] CE; //计数器工作单元:只用低8位  
  
    //读写信号：均为高电平有效
    wire wmode;//写工作方式字
    wire wnowcount;//锁存当前计数值
    wire wrback;//写读回命令
    wire rcounter0;//读计数器0
    wire wcounter0;//写计数器0  
    wire wcr;//写初始值寄存器  
    /*因为写计数器0的工作模式方式字,所以根据P241表8.1先满足写控制字寄存器写控制字的条件，再根据P246满足选择计数器0，写低8位初值。
    所以有csn:0 IOR_N:1 IOW_N:0 a0:1 a1:0 D7D6=00 D5D4=01 ，逻辑描述如下：*/
    assign wmode = (!CS_N) & (IOR_N) & (!IOW_N) & (a[1]) & (a[0]) & (!id[7]) & (!id[6]) & (!id[5]) & (id[4]);
    assign wnowcount = (!CS_N) & (IOR_N) & (!IOW_N) & (a[1]) & (a[0]) & (!id[7]) & (!id[6]) & (!id[5]) & (!id[4]);
    assign wrback = (!CS_N) & (IOR_N) & (!IOW_N) & (a[1]) & (a[0]) & (id[7]) & (id[6]) & (!id[3]) & (!id[2]) & (id[1]) & (!id[0]);
    assign rcounter0 = (!CS_N) & (!IOR_N) & (IOW_N) & (!a[1]) & (!a[0]);
    assign wcounter0 = (!CS_N) & (IOR_N) & (!IOW_N) & (!a[1]) & (!a[0]);
    //此处还应填写对计数初值寄存器可赋值的前提条件，条件应满足：       
    assign wcr = wcounter0 & (((SR[0]==1'b0) || //1、二进制计数时，输入范围为0x00~0xff
                              (SR[0] == 1'b1 && id[7:4] <= 4'd9 && id[3:0] <= 4'd9)) &&  //2、BCD码计数时，输入范围应符合BCD数据范围要求
                             (SR[2:1]!=2'b11 || id!=8'h01));//3、方式3时，初值不能为1
     
    reg [5:0] MR;//工作方式寄存器
    //在写工作模式方式字信号的上升沿，对工作方式寄存器赋值，因为只有低6位存有相关工作方式的信息。
    always @ (posedge wmode) begin
        MR <=  id[5:0];
    end 
  
    //状态寄存器SR:根据P247页图8.11完成状态寄存器的赋值
    always @ (*) begin
      SR = {out0,NULL_COUNT,MR[5:0]};
    end
    
    //计数初值寄存器：CR
    reg CRinitflag;//有效表示初值已写入,高电平有效。    
    always @ (posedge wcr or posedge wmode) begin//写计数器初值或者写方式字时被触发
        if(wmode) begin
            CR <= 16'h0000;//完成清0操作
            CRinitflag <= 0;            
        end else begin 
            CR <= {8'h00,id}; //写入计数器初值
            CRinitflag <= 1;                
        end 
    end
      
    reg Rflag;//rising edge flag用以标记GATE0上升沿
    reg Rtrace;//用以跟踪Rflag
    always@(posedge gate0 or posedge wmode)begin
        if(wmode) begin
            Rflag <= 0;
        end else begin
            if(Rtrace==Rflag)
                Rflag <= ~Rflag;
        end
    end
  
    reg NULL_COUNT;//计数值是否有效：1--无效 0--有效
    reg out_temp;  //方式3时的out输出
    reg OEflag;    //用以表示方式3下计数初值的奇偶情况，0为偶数，1为奇数
    reg Fwcr;      //用以表示方式3，写入方式字后，第一次写入初值
    reg FWcr1;     //用于防止持续写初值造成的无法记录上升沿，针对所有方式表示第一个初值已被写入。
    reg zflag;     //用于方式1，为1时表示计数开始，为0时表示计数正在进行或者结束
    //控制方式1和方式3下的CE计数，产生方式三下的out输出 
    always@(negedge clk0 or posedge wcr or posedge wmode) begin
        if(wmode)begin //写入工作方式控制字时，逻辑复位
            NULL_COUNT <= 1;
            out_temp <= 1;
            Fwcr <= 1'b1;FWcr1 <= 1'b1;CE <= 0;  
        end else if(wcr)begin//当发生写初值时或者初值未写入
            NULL_COUNT <= 1;
            if(FWcr1 == 1)begin //只追踪第一次写入初值后的上升沿，防止重复写初值造成的覆盖上升沿
                zflag  <= 0;    //初值未重装，不开始计数。
                Rtrace <= Rflag; 
                FWcr1 <= 0; 
            end
        end else if (!CRinitflag)begin
                //写方式字后，当初值未写入时，不做任何处理            
        end else if(SR[3:1] == 3'b000)begin//方式0：无重装，计数过程中改变计数初值立即有效，GATE==0 禁止计数，上升沿继续计数
            if(NULL_COUNT == 1)begin//写入新值马上按新初值重新计数
                NULL_COUNT <= 0;
                CE <= CR;
                zflag <= 1;
            end else begin//初值已写入CE
                if(gate0 == 1'b1) begin//gate0高电平计数，低电平暂停计数
                    if(CE > 16'h0000 || zflag == 1)begin
                        if(SR[0] == 1'b1) begin //对于BCD码计数
                            if (CE==0) begin
                                CE[7:4] <= 4'h9;CE[3:0] <= 4'h9;
                            end else if ((CE[3:0] == 4'h0) && (CE[7:4] > 4'h0)) begin
                                CE[7:4] <= CE[7:4] - 4'h1;CE[3:0] <= 4'h9;
                            end else begin
                                CE[7:0] <= CE[7:0] - 8'h01;
                            end
                        end else begin
                            CE[7:0] <= CE[7:0] - 8'h01;
                        end
                        zflag <= 0;
                    end else begin
                        CE <= 0; //计数到0停止计数
                    end
                end 
            end
        end else if((SR[3:1] == 3'b001)||(SR[3:1] == 3'b101))begin//方式1、方式5：无重装，计数过程中改变计数初值GATE触发后有效
            if(Rtrace != Rflag) begin //发生了gate0上升沿，从计数初值开始计数，否则不受影响
                NULL_COUNT <= 0;
                Rtrace <= Rflag;
                CE <= CR;
                zflag <= 1;
            end else if(CE > 16'h0000 || zflag==1) begin
                if(SR[0] == 1'b1) begin //对于BCD码计数
                    if(CE==0) begin
                        CE[7:4] <= 4'h9;CE[3:0] <= 4'h9;
                    end else if ((CE[3:0] == 4'h0) && (CE[7:4] > 4'h0)) begin
                        CE[7:4] <= CE[7:4] - 4'h1;
                        CE[3:0] <= 4'h9;
                    end else begin
                        CE[7:0] <= CE[7:0] - 8'h01;
                    end
                end else begin
                  CE[7:0] <= CE[7:0] - 8'h01;
                end
                zflag <= 0;
                if(CE == 1) begin//只针对方式5，根据非阻塞赋值，当CE为1时，out2变0，保持一个周期变一
                    out_temp <= 0;
                end else begin
                    out_temp <= 1;
                end
            end else begin
                out_temp <= 1;
                CE <= 0;
            end
        end else if(SR[2:1] == 2'b10) begin //方式二：计数到1重装，GATE上升沿重装，GATE==0禁止计数
            if(Fwcr == 1) begin
                Fwcr <= 0;
                CE<= CR;
                NULL_COUNT <= 0;
            end else if(Rtrace != Rflag) begin//上升沿重装初始值,并更新NULL_COUNT
                NULL_COUNT <= 0;
                Rtrace <= Rflag;
                CE <= CR;
            end else if (gate0 == 1'b1) begin //为1时开始计数，为0时停止计数
                if(CE == 16'h0001)begin
                    CE <= CR;
                    NULL_COUNT <= 0;
                    out_temp <= 1'b1;
                end else if(SR[0] == 1'b1) begin //对于BCD码计数
                    if((CE[3:0] == 4'h0) && (CE[7:4] == 4'h0)) begin
                        CE[7:4] <= 4'h9;
                        CE[3:0] <= 4'h9;
                    end else if ((CE[3:0] == 4'h0) && (CE[7:4] > 4'h0)) begin
                        CE[7:4] <= CE[7:4] - 4'h1;
                        CE[3:0] <= 4'h9;
                    end else begin
                        CE[7:0] <= CE[7:0] - 8'h01;
                    end
                end else begin
                    CE[7:0] <= CE[7:0] - 8'h01;
                end                 
                if(CE==2) begin
                    out_temp <= 1'b0;
                end else begin
                    out_temp <= 1'b1;
                end
            end
        end else if(SR[2:1] == 2'b11) begin//方式3：计数到2重装，GATE上升沿重装，GATE==0停止计数,请自行完成方式3
            if(Fwcr == 1) begin
                Fwcr <= 0;
                CE <= {CR[15:1],1'b0};
                OEflag <= CR[0];
                NULL_COUNT <= 0;
            end else if(Rtrace != Rflag) begin//上升沿重装初始值
                Rtrace <= Rflag;
                NULL_COUNT <= 0;
                CE<= {CR[15:1],1'b0};
                out_temp <= 1;
                OEflag <=  CR[0];
            end else if (gate0 == 1'b1) begin //为1时开始计数，为0时停止计数
                if(OEflag==0 && CE == 16'h0002)begin
                    CE <= {CR[15:1],1'b0};
                    OEflag <= CR[0];
                    NULL_COUNT <= 0;
                    out_temp <= ~out_temp;//计数到时反向
                end else if(OEflag==1 &&((CE==16'h0002 && out_temp==0)||(CE==16'h0000 && out_temp==1)))begin 
                    CE <= {CR[15:1],1'b0};
                    OEflag <= CR[0];
                    NULL_COUNT <= 0;
                    out_temp <= ~out_temp;//计数到时反向    
                end else if(SR[0] == 1'b1) begin //对于BCD码计数
                    if((CE[3:0] == 4'h0) && (CE[7:4] == 4'h0)) begin
                        CE[7:4] <= 4'h9;
                        CE[3:0] <= 4'h8;
                    end else if ((CE[3:0] == 4'h0) && (CE[7:4] > 4'h0)) begin
                        CE[7:4] <= CE[7:4] - 1;
                        CE[3:0] <= 4'h8;
                    end else begin
                        CE[7:0] <= CE[7:0] - 8'h02;
                    end
                end else begin
                    CE[7:0] <= CE[7:0] - 8'h02;
                end
            end
        end else if(SR[3:1] == 3'b100) begin//方式4：计数过程中改变初值立即有效，GATE上升沿重装，GATE==0停止计数
            if(Rtrace != Rflag) begin //发生了gate0上升沿，从计数初值开始计数，否则不受影响
                NULL_COUNT <= 0;
                Rtrace <= Rflag;
                CE <= CR;
                zflag <= 1;
            end else if(NULL_COUNT == 1)begin
                CE <= CR;
                NULL_COUNT <= 0;
                zflag <= 1;
            end else if (gate0 == 1'b1) begin //为1时开始计数，为0时停止计数 
                if(CE > 16'h0000 || zflag==1) begin
                    if(SR[0] == 1'b1) begin //对于BCD码计数
                        if (CE==0) begin
                            CE[7:4] <= 4'h9;CE[3:0] <= 4'h9;
                        end else if ((CE[3:0] == 4'h0) && (CE[7:4] > 4'h0)) begin
                            CE[7:4] <= CE[7:4] - 4'h1;CE[3:0] <= 4'h9;
                        end else begin
                            CE[7:0] <= CE[7:0] - 8'h01;
                        end
                    end else begin
                        CE[7:0] <= CE[7:0] - 8'h01;
                    end
                    zflag <= 0;
                    if(CE == 1) begin
                        out_temp <= 0;
                    end else begin
                        out_temp <= 1;
                    end
                end else begin
                    out_temp <= 1;
                    CE <= 0;
                end
            end
        end        
    end 
    
    //out0产生逻辑：方式0、1、2适合用组合逻辑，不涉及到时序，方式3、4、5涉及时序
    always @ (*) begin
        if(wmode) begin
            if(id[3:1] == 3'b000) begin
                out0 = 0;
            end else begin
                out0 = 1;
            end
        end else if(SR[3:1] == 3'b000) begin
            if((CE == 16'h0000)&&(NULL_COUNT==0)&&(zflag==0)) begin
                out0 = 1'b1;
            end else begin //重写控制字或者初值
                out0 = 1'b0;
            end
        end else if(SR[3:1] == 3'b001)begin
            if(CE == 16'h0000&&(zflag==0)) begin
                out0 = 1'b1;
            end else begin//触发后变0
                out0 = 1'b0;
            end
        end else begin
            out0 = out_temp;
        end
    end
  
    //SL：状态锁存器
    always @ (posedge wmode or posedge wrback)begin
        if(wmode) begin
           SL <= 0;
        end else if(wrback & !id[4])begin
           SL <= SR;
        end else begin
           SL <= SL;
        end
    end  
  
    //OL：完成输出锁存器相关逻辑
    always @ (posedge wmode or posedge wnowcount or posedge wrback) begin
        if(wmode) begin
            OL <= 0;
        end else if(wnowcount) begin 
            OL <= CE;
        end else if(wrback & !id[5]) begin//防止编译器问题，分开写
            OL <= CE;
        end else begin
            OL <= OL;
        end
    end  
    
    //od 
    reg [1:0] rsta;//[0]表示读状态；[1]表示读低8位； [2]表示读高8位；
    always @ (posedge wrback or posedge wnowcount or posedge wmode) begin
        if(wmode) begin
            rsta <= 2'b00;
        end else if(wnowcount)begin
            rsta <= 2'b10;
        end else begin
            rsta <= ~id[5:4];       
        end 
    end    
     
    reg rstaflag;    
    wire rstaflagreset;
    assign rstaflagreset = wmode | wnowcount | wrback;     
    always @ (posedge rstaflagreset or posedge rcounter0) begin
        if(rstaflagreset) begin
            rstaflag <= 0;
            od       <= 8'h00;
        end else begin
            case (rsta)
                2'b10:  od <= OL[7:0];
                2'b01:  od <= SL; 
                2'b11: begin 
                    if(!rstaflag) begin
                      od <= SL; rstaflag <= 1;
                    end else begin
                      od <= OL[7:0]; 
                    end             
                end
                2'b00:   od <= 0; 
                default: od <= 0;
            endcase 
        end
    end    
 
endmodule
