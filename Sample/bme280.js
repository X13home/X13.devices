class BME280{
  constructor(){
    this.st=new UInt8(0);
    this.T=new Int16();
    this.H=new UInt16();
    this.P=new Int32();
    this.present=false;
    
    this.dig_T1=new UInt16();
    this.dig_T2=new Int16();
    this.dig_T3=new Int16();
    this.dig_P1=new UInt16();
    this.dig_P2=new Int16();
    this.dig_P3=new Int16();
    this.dig_P4=new Int16();
    this.dig_P5=new Int16();
    this.dig_P6=new Int16();
    this.dig_P7=new Int16();
    this.dig_P8=new Int16();
    this.dig_P9=new Int16();
    this.dig_H1=new UInt8();
    this.dig_H2=new Int16();
    this.dig_H3=new UInt8();
    this.dig_H4=new Int16();
    this.dig_H5=new Int16();
    this.dig_H6=new Int8();
  
    this.to=0;
  }
  Read(){
    let var0=this.st, var1, var2;
    if(var0==8){                                    // 107 uS
      if((var1=getSeconds())>=this.to){             // 150 uS
        this.to=var1+15;
        this.st=2;      
      }        
    } else if(var0<5 && (var1=TwiStatus())==0){
      if(var0==0){
        TwiControl(0x1A010376);
        TwiPutByte(0x88);
        this.st=5;
      } else if(var0==1){
        this.st=6;
        TwiControl(0x07010376);
        TwiPutByte(0xE1);
      } else if(this.st==2){                        // 303 uS
        this.st=3;
        TwiControl(0x00040176);		                // Pressure oversampling x1, Temperature oversampling x1, Humidity oversampling x1
        TwiPutByte(0xF2);
        TwiPutByte(0x01);
        TwiPutByte(0x00);
        TwiPutByte(0x24);
      } else if(this.st==3){                        // 323 uS
        this.st=4;
        TwiControl(0x00020176);		                // forced mode
        TwiPutByte(0xF4);
        TwiPutByte(0x25);
      } else{                                       // 312 uS
        this.st=7;
        TwiControl(0x08010376);
        TwiPutByte(0xF7);
      }
    } else if( (var1=TwiStatus() & 0xFFFF)==0x1076 ){
      if(var0==5){
        this.dig_T1=TwiGetByte() | (TwiGetByte()<<8);
        this.dig_T2=TwiGetByte() | (TwiGetByte()<<8);
        this.dig_T3=TwiGetByte() | (TwiGetByte()<<8);
        this.dig_P1=TwiGetByte() | (TwiGetByte()<<8);
        this.dig_P2=TwiGetByte() | (TwiGetByte()<<8);
        this.dig_P3=TwiGetByte() | (TwiGetByte()<<8);
        this.dig_P4=TwiGetByte() | (TwiGetByte()<<8);
        this.dig_P5=TwiGetByte() | (TwiGetByte()<<8);
        this.dig_P6=TwiGetByte() | (TwiGetByte()<<8);
        this.dig_P7=TwiGetByte() | (TwiGetByte()<<8);
        this.dig_P8=TwiGetByte() | (TwiGetByte()<<8);
        this.dig_P9=TwiGetByte() | (TwiGetByte()<<8);
        var1=TwiGetByte();
        this.dig_H1=TwiGetByte();
        this.st=1;
      } else if(var0==6){
        this.st=2;
        this.dig_H2=TwiGetByte() | (TwiGetByte()<<8);
        this.dig_H3=TwiGetByte();
        var1=TwiGetByte();
        var2=TwiGetByte();
        this.dig_H4=(var1 << 4) | var2 & 0x0F;
        this.dig_H5=(var2 >> 4) | (TwiGetByte()<<4);
        this.dig_H6=TwiGetByte();
      } else if(this.st==7){                                    // 1,2 mS
	    let ADC1, ADC2, T_Fine;
	    ADC1=(TwiGetByte()<<12) | (TwiGetByte()<<4) | (TwiGetByte()>>4);
	    ADC2=(TwiGetByte()<<12) | (TwiGetByte()<<4) | (TwiGetByte()>>4);

    	var1 = (((ADC2>>3) - (this.dig_T1<<1)) * (this.dig_T2) ) >> 11;
    	var2 = (((((ADC2>>4) - (this.dig_T1)) * ((ADC2>>4) - (this.dig_T1))) >> 12) * (this.dig_T3)) >> 14;
    	T_Fine = var1 + var2;
    	this.T=(T_Fine * 5 + 128) >> 8;  // temperature in DegC, resolution is 0.01 DegC

    	var1 = (T_Fine>>1)-64000;
	    var2 = (((var1>>2) * (var1>>2)) >> 11 ) * this.dig_P6;
    	var2 = var2 + ((var1*this.dig_P5)<<1);
    	var2 = (var2>>2)+(this.dig_P4<<16);
    	var1 = (((this.dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((this.dig_P2 * var1)>>1))>>18;
    	var1 =((32768+var1)*this.dig_P1)>>15;
    	if(var1 != 0){
      	  let var3 = (1048576-ADC1-(var2>>12))*3125;
          var3 = (var3/var1)*2;
          var1 = (this.dig_P9 * (((var3>>3) * (var3>>3))>>13))>>12;
          var2 = ((var3>>2) * this.dig_P8)>>13;
          this.P = (var3 + ((var1 + var2 + this.dig_P7) >> 4)); // pressure in Pa
        }

    	ADC2=TwiGetByte()<<8 | TwiGetByte();
    	var1=T_Fine - 76800;
    	var1 = (((((ADC2 << 14) - (this.dig_H4 << 20) - (this.dig_H5 * var1)) + 16384) >> 15) 
    	  * (((((((var1 * this.dig_H6) >> 10) * (((var1 * this.dig_H3) >> 11) + 32768)) >> 10) + 2097152) * this.dig_H2 + 8192) >> 14));
    	var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * this.dig_H1) >> 4))>>13;
    	this.H = (var1 < 0 ? 0 : ((var1 > 51200) ? 51200 : var1));  // humidity in %RH = this.H/512.0

        this.st=8;
        this.present=true;
      }
    } else if((var1 & 0xFF)==0x76 && (var1 & 0xFF00)!=0){
      this.st=0;
      this.present=false;
    }
  }  
}

var bme=new BME280();
bme.Read();
