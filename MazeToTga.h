
#ifndef __MAZE__TO__TGA__0_
#define __MAZE__TO__TGA__0_ 1
#ifndef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS 1
#endif
#include"Maze.h"
#include"TGA.h"

class MazeToTga
{
public:
	typedef TGA::Pixel pixel_t;
	typedef pixel_t color_t;
	typedef TGA::Pen pen_t;
	/*页边距*/
	std::uint16_t margin;
	/*每个正方网格大小*/
	std::uint16_t cellSize ;
	/*背景颜色*/
	color_t backgroundColor;
	/*画笔大小及颜色*/
	pen_t pen;
	/*构造函数*/
	MazeToTga(
		std::uint16_t i_margin = 15, 
		std::uint16_t i_cellSize = 15, 
		color_t i_bcolor = { 90, 91, 92 }, 
		pen_t i_pen = pen_t()
		) :margin(i_margin),
	cellSize(i_cellSize),
	backgroundColor(i_bcolor),
	pen(i_pen){}
	/**/
	TGA convert(const Maze & maze)
	{
		typedef uint16_t u16;
		uint16_t _w_ = maze.getWidth()*cellSize + 2 * margin;
		uint16_t _h_ = maze.getHeight()*cellSize + 2 * margin;
		TGA tga(_w_,_h_);
		/*设置背景颜色*/
		for (u16 ii = 0; ii < tga.getWidth();++ii)
		{
			for (u16 jj = 0; jj < tga.getHeight();++jj)
			{
				tga.getPixel(ii,jj) = backgroundColor;
			}
		}
		/*设置画笔*/
		tga.getPen() = pen;
		/*绘制横线*/
		for (u16 jj = 0; jj < maze.getHeight();++jj)
		{
			for (u16 ii = 0; ii < maze.getWidth();++ii)
			{
				if (maze.getRowMatrix().get(ii,jj))
				{
					u16 _px = margin + ii*cellSize;
					u16 _py = margin + jj*cellSize+cellSize;
					tga.draw(_px, _py, _px + cellSize, _py);
				}
			}
		}
		/*绘制纵线*/
		for (u16 ii = 0; ii < maze.getWidth(); ++ii)
		{
			for (u16 jj = 0; jj < maze.getHeight(); ++jj)
			{
				if (maze.getColumnMatrix().get(ii,jj))
				{
					u16 _px = margin + ii*cellSize + cellSize;
					u16 _py = margin + jj*cellSize;
					tga.draw(_px, _py, _px, _py + cellSize);
				}
			}
		}
		/*绘制边界线*/
		tga.draw(margin, margin, margin, _h_ -  margin);
		tga.draw(margin, margin, _w_ - margin, margin);
		tga.draw(margin, _h_ - margin, _w_ - margin, _h_ - margin);
		tga.draw(_w_ - margin, margin, _w_ - margin, _h_ - margin);
		/*绘制开口*/
		tga.getPen().pixel=backgroundColor;
		tga.draw(margin, margin, margin, cellSize+margin);
		tga.draw(_w_ - margin, _h_ - margin*2, _w_ - margin, _h_ - margin );
		return std::move(tga);
	}
};

#endif