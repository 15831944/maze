#ifndef __TIFF__TGA__H__
#define __TIFF__TGA__H__ 1
#ifndef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS 1
#endif

#include<cstdint>
#include<cstddef>
#include<string>
#include<fstream>
#include<algorithm>

namespace __TGA__ns{

	struct TIFF
	{
		struct Pixel
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
			Pixel(unsigned char i_r = 125, unsigned char i_g = 126, unsigned char i_b = 127) :r(i_r), g(i_g), b(i_b){}
		};
		struct Pen
		{
			Pixel pixel;/*color*/
			unsigned char width;
			Pen(unsigned char i_width = 3, Pixel i_pixel = Pixel{ 0, 0, 0 }) :pixel(i_pixel), width(i_width){}
		};
		/*像素访问器*/
		Pixel & getPixel(std::uint16_t x, std::uint16_t y)
		{
			static Pixel _p_;
			if (height_ > y&&pixel_)
			{
				return pixel_[(height_ - y - 1)*width_ + x];
			}
			return _p_;
		}
		const Pixel & getPixel(std::uint16_t x, std::uint16_t y)const
		{
			static Pixel _p_;
			if (height_ > y&&pixel_)
			{
				return pixel_[(height_ - y - 1)*width_ + x];
			}
			return _p_;
		}
		/*画笔访问器*/
		Pen & getPen(){ return pen_; }
		const Pen & getPen()const { return pen_; }
		/*画布大小*/
		std::uint16_t getWidth()const{ return width_; }
		std::uint16_t getHeight()const{ return height_; }
		/*
		绘制直线
		*/
		void draw(std::uint16_t ix, std::uint16_t iy){ draw(ix, iy, ix, iy); }
		void draw(std::uint16_t ix, std::uint16_t iy, std::uint16_t jx, std::uint16_t jy)
		{
			if (pen_.width == 0 || ix >= width_ || jx >= width_ || iy >= height_ || jy >= height_){ return; }
			if ((ix == jx) && (iy == jy)){ _draw_point(ix, iy); return; }
			if ((ix == jx) || (iy == jy)){ _draw_line(ix, iy, jx, jy); return; }
			_draw_points(ix, iy, jx, jy);
		}
		TIFF(std::uint16_t i_width = 100, std::uint16_t i_height = 100) :width_(i_width), height_(i_height)
		{
			pixel_ = new Pixel[width_*height_];
		}
		~TIFF()
		{
			delete[]pixel_;
		}
		TIFF(TIFF && i_tiff)
		{
			this->width_ = i_tiff.width_;
			this->height_ = i_tiff.height_;
			this->pixel_ = i_tiff.pixel_;
			i_tiff.pixel_ = 0;
		}
		TIFF(const TIFF & i_tiff)
		{
			this->width_ = i_tiff.width_;
			this->height_ = i_tiff.height_;
			pixel_ = new Pixel[width_*height_];
			if (pixel_)
				std::copy(i_tiff.pixel_, i_tiff.pixel_ + width_*height_, pixel_);
		}
		TIFF & operator=(TIFF && i_tiff)
		{
			delete[]pixel_;
			this->width_ = i_tiff.width_;
			this->height_ = i_tiff.height_;
			this->pixel_ = i_tiff.pixel_;
			i_tiff.pixel_ = 0;
			return *this;
		}
		TIFF & operator=(const TIFF & i_tiff)
		{
			if (this != &i_tiff)
			{
				if (width_*height_ != i_tiff.width_*i_tiff.height_)
				{
					delete[]pixel_;
					pixel_ = new Pixel[i_tiff.width_*i_tiff.height_];
				}
				this->width_ = i_tiff.width_;
				this->height_ = i_tiff.height_;
				if (pixel_)
					std::copy(i_tiff.pixel_, i_tiff.pixel_ + width_*height_, pixel_);
			}
			return *this;
		}
		void save(const std::string & i_fileName = "ans")
		{
			_writeTIFFBin(i_fileName.c_str(), (char *)(pixel_), sizeof(Pixel)*width_*height_);
		}
	private:

		/*二进制写tiff文件*/
		void _writeTIFFBin(
			const char * i_fileName,
			const char * i_data,
			const std::size_t i_dataSize)
		{
			if (i_fileName == 0 || i_data == 0 || i_dataSize == 0)
			{
				return;
			}
			/*tiff头 参考opengl超级宝典5*/
			typedef unsigned char GLbyte;
#pragma pack(1)
			struct TGAHEADER
			{
				/*1*/GLbyte	identsize;                      // Size of ID field that follows header (0)
				/*1*/GLbyte	colorMapType;                   // 0 = None, 1 = paletted
				/*1*/GLbyte	imageType;                      // 0 = none, 1 = indexed, 2 = rgb, 3 = grey, +8=rle
				/*2*/std::uint16_t	colorMapStart;          // First colour map entry
				/*2*/std::uint16_t	colorMapLength;         // Number of colors
				/*1*/unsigned char 	colorMapBits;           // bits per palette entry
				/*2*/std::uint16_t	xstart;                 // image x origin
				/*2*/std::uint16_t	ystart;                 // image y origin
				/*2*/std::uint16_t	width;                  // width in pixels
				/*2*/std::uint16_t	height;                 // height in pixels
				/*1*/GLbyte	bits;                           // bits per pixel (8 16, 24, 32)
				/*1*/GLbyte	descriptor;                     // image descriptor
			};
#pragma pack()
			static_assert(sizeof(TGAHEADER) == 18, "core error!");
			TGAHEADER tgaHeader;		// TGA file header
			// Initialize the Targa header
			tgaHeader.identsize = 0;
			tgaHeader.colorMapType = 0;
			tgaHeader.imageType = 2;
			tgaHeader.colorMapStart = 0;
			tgaHeader.colorMapLength = 0;
			tgaHeader.colorMapBits = 0;
			tgaHeader.xstart = 0;
			tgaHeader.ystart = 0;
			tgaHeader.width = this->width_;
			tgaHeader.height = this->height_;
			tgaHeader.bits = 24;
			tgaHeader.descriptor = 0;
			//
			std::ofstream ofs(std::string(i_fileName) + std::string(".tga"), std::ios::binary | std::ios::out);
			if (ofs.is_open())
			{
				ofs.write((char *)(&tgaHeader), sizeof(tgaHeader));
				ofs.write(i_data, i_dataSize);
			}
		}
		std::uint16_t	width_ = 0;
		std::uint16_t	height_ = 0;
		Pixel * pixel_ = 0;
		Pen pen_;
		void _fillRect(std::uint16_t x1, std::uint16_t y1, std::uint16_t x2, std::uint16_t y2)
		{
			if (x2 >= x1&&y2 >= y1)
			{
				const std::uint16_t _y1_ = y1;
				for (; x1 < x2; ++x1)
				{
					for (y1 = _y1_; y1 < y2; ++y1)
					{
						getPixel(x1, y1) = pen_.pixel;
					}
				}
			}
		}
		void _draw_point(std::uint16_t x, std::uint16_t y)
		{
			if (pen_.width == 1){ getPixel(x, y) = pen_.pixel; return; }
			std::uint16_t _width_ = pen_.width / 2;
			std::uint16_t left_top_x = x > _width_ ? (x - _width_) : 0;
			std::uint16_t left_top_y = y > _width_ ? (y - _width_) : 0;
			std::uint16_t right_bot_x = x + _width_;
			std::uint16_t right_bot_y = y + _width_;
			if (right_bot_x >= width_){ right_bot_x = width_ - 1; }
			if (right_bot_y >= height_){ right_bot_y = height_ - 1; }
			_fillRect(left_top_x, left_top_y, right_bot_x, right_bot_y);
		}
		void _draw_line(std::uint16_t ix, std::uint16_t iy, std::uint16_t jx, std::uint16_t jy)
		{
			if (iy > jy || ix > jx){ return; }
			std::uint16_t _width_ = pen_.width / 2;
			std::uint16_t left_top_x = ix > _width_ ? (ix - _width_) : 0;
			std::uint16_t left_top_y = iy > _width_ ? (iy - _width_) : 0;
			std::uint16_t right_bot_x = ix + _width_;
			std::uint16_t right_bot_y = iy + _width_;
			if (ix == jx)
			{
				right_bot_y += jy - iy;
			}
			else
			{
				right_bot_x += jx - ix;
			}
			if (right_bot_x >= width_){ right_bot_x = width_ - 1; }
			if (right_bot_y >= height_){ right_bot_y = height_ - 1; }
			_fillRect(left_top_x, left_top_y, right_bot_x, right_bot_y);
		}
		void _draw_points(std::uint16_t ix, std::uint16_t iy, std::uint16_t jx, std::uint16_t jy)
		{
			float _k_ = float(jy - iy) / float(jx - ix);
			float _b_ = iy - _k_*ix;
			if (_k_ > 1)
			{
				_k_ = float(jx - ix) / float(jy - iy);
				_b_ = ix - _k_*iy;
				for (; iy < jy; ++iy)
				{
					ix = std::uint16_t(_k_*iy + _b_);
					_draw_point(ix, iy);
				}
			}
			else
			{
				for (; ix < jx; ++ix)
				{
					iy = std::uint16_t(_k_*ix + _b_);
					_draw_point(ix, iy);
				}
			}
		}
		static inline void LITTLE_ENDIAN_WORD(std::uint16_t * ix)
		{
			typedef unsigned char _X_[2];
			_X_ * _x_ = (_X_ *)(ix);
			std::swap((*_x_)[0], (*_x_)[1]);
		}
	};//~class

#ifdef _TGA_TEST_
	int main()
	{
		TIFF tiff;
		tiff.draw(80,80);
		tiff.draw(20, 30,20,80);
		tiff.draw(50, 60, 60, 60);
		tiff.draw(10, 10, 60, 20);
		tiff.draw(10, 10, 60, 95);
		tiff.save();
	}
#endif
}//~namespace
#endif