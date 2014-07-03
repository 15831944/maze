
#ifndef __MAZE__H_0_
#define __MAZE__H_0_
#ifndef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS 1
#endif
#include<cstdint>
#include<cstddef>
#include<memory>
#include<fstream>
#include<string>
#include<sstream>

class Maze
{
	typedef std::uint16_t u16;
	typedef std::size_t st;

public:
	typedef Maze ThisType;
	/*------------------------------------------------------*/
	/*
	一个小型二维矩阵
	尽量少做安全检查
	*/
	template<typename T,typename sizeType=u16>
	struct Matrix2
	{
		typedef sizeType s_t;
		typedef Matrix2 ThisType;
		Matrix2(sizeType i_width=100, sizeType i_height=100):width(i_width),height(i_height)
		{
			data = new T[width*height];
		}
		~Matrix2(){delete[]data;}
		Matrix2(ThisType && m2)
		{
			width = m2.width;
			height = m2.height;
			data = m2.data;
			m2.data = 0;
		}
		Matrix2(const ThisType & m2) :Matrix2(m2.width,m2.height)
		{
			if (data)
			{
				for (s_t ii = 0; ii < width*height;++ii)
				{
					data[ii] = m2.data[ii];
				}
			}
			else
			{
				width = 0;
				height = 0;
			}
		}
		ThisType & operator=(const ThisType & m2)
		{
			if (this == &m2){ return *this; }
			if (width*height!=(m2.width*m2.height))
			{
				delete[]data;
				data = new T[m2.width*m2.height];
			}
			if (data)
			{
				width = m2.width;
				height = m2.height;
				for (s_t ii = 0; ii < width*height; ++ii)
				{
					data[ii] = m2.data[ii];
				}
			}
			else
			{
				width = 0;
				height = 0;
			}
			return *this;
		}
		ThisType & operator=(ThisType && m2)
		{
			delete[]data;
			width = m2.width;
			height = m2.height;
			data = m2.data;
			m2.data = 0;
			return *this;
		}
		T & get(sizeType x,sizeType y)
		{
			return data[y*width + x];
		}
		const T & get(sizeType x, sizeType y) const
		{
			return data[y*width + x];
		}
		void setTo(const T &value)
		{
			if (data)
			for (s_t ii = 0; ii < width*height; ++ii)
			{
				data[ii] = value;
			}
		}
		s_t width;
		s_t height;
		T * data=0;
	};
	/*------------------------------------------------------*/
	Maze(std::uint16_t i_width=10,std::uint16_t i_height=10):rowMatrix_(i_width,i_height),
	columnMatrix_(i_width,i_height),
	maskMatrix_(i_width,i_height)
	{
		/*初始化全部连通*/
		rowMatrix_.setTo(1);
		columnMatrix_.setTo(1);
		/*初始化标记全部未遍历*/
		maskMatrix_.setTo(0);
	}
	Maze(Maze && m)
	{
		rowMatrix_ = std::move(m.rowMatrix_);
		columnMatrix_ = std::move(m.columnMatrix_);
		maskMatrix_ = std::move(m.maskMatrix_);
	}
	Maze(const Maze & m)
	{
		rowMatrix_ = m.rowMatrix_;
		columnMatrix_ = m.columnMatrix_;
		maskMatrix_ = m.maskMatrix_;
	}
	ThisType & operator=(const ThisType & m)
	{
		if (this == &m){ return *this; }
		rowMatrix_ = m.rowMatrix_;
		columnMatrix_ = m.columnMatrix_;
		maskMatrix_ = m.maskMatrix_;
		return *this;
	}
	ThisType & operator=(ThisType && m)
	{
		rowMatrix_ = std::move(m.rowMatrix_);
		columnMatrix_ = std::move(m.columnMatrix_);
		maskMatrix_ = std::move(m.maskMatrix_);
		return *this;
	}
	//
	std::uint16_t getWidth()const{ return rowMatrix_.width; }
	std::uint16_t getHeight()const{ return columnMatrix_.height; }
	//
	Matrix2<unsigned char> & getRowMatrix(){ return rowMatrix_; }
	const Matrix2<unsigned char> & getRowMatrix()const{ return rowMatrix_; }
	Matrix2<unsigned char> & getColumnMatrix(){ return columnMatrix_; }
	const Matrix2<unsigned char> & getColumnMatrix()const{ return columnMatrix_; }
	Matrix2<unsigned char> & getMaskMatrix(){ return maskMatrix_; }
	const Matrix2<unsigned char> & getMaskMatrix()const{ return maskMatrix_; }
	//
	void save(const std::string & i_fileName="ans.txt")const{ _save(i_fileName); }
	void load(const std::string & i_fileName = "ans.txt"){ _load(i_fileName); }
private:
	/*纵向联通矩阵*/
	Matrix2<unsigned char> rowMatrix_;
	/*横向联通矩阵*/
	Matrix2<unsigned char> columnMatrix_;
	/*是否遍历蒙版矩阵*/
	Matrix2<unsigned char> maskMatrix_;
	//
	void _save(const std::string & i_fileName)const
	{
		std::ofstream ofs(i_fileName);
		if (ofs.is_open())
		{
			/*大小*/
			ofs << "size:" << std::endl;
			ofs<< getWidth() << " " << getHeight() << std::endl;
			/*是否遍历蒙版矩阵*/
			ofs << "mask:" << std::endl;
			for (u16 jj = 0; jj < getHeight(); ++jj)
			{
				for (u16 ii = 0; ii < getWidth(); ++ii)
				{
					ofs<< u16(maskMatrix_.get(ii,jj))<<" ";
				}
				ofs << std::endl;
			}
			
			/*横向联通矩阵*/
			ofs << "column:" << std::endl;
			for (u16 jj = 0; jj < getHeight(); ++jj)
			{
				for (u16 ii = 0; ii < getWidth(); ++ii)
				{
					ofs << u16(columnMatrix_.get(ii, jj)) << " ";
				}
				ofs << std::endl;
			}
			/*纵向联通矩阵*/
			ofs << "row:" << std::endl;
			for (u16 jj = 0; jj < getHeight(); ++jj)
			{
				for (u16 ii = 0; ii < getWidth(); ++ii)
				{
					ofs << u16(rowMatrix_.get(ii, jj)) << " ";
				}
				ofs << std::endl;
			}
		}
	}//
	void _load(const std::string & i_fileName = "ans.txt")
	{
		std::ifstream ifs(i_fileName);
		std::string line;
		if (ifs.is_open())
		{
			ThisType * maze=0;
			while (std::getline(ifs,line))
			{
				if (line.size())
				{
					/*------------------------------------*/
					if (line=="size:")
					{
						std::getline(ifs, line);
						if (line.size())
						{
							u16 _w=0; 
							u16 _h=0;
							std::stringstream ss;
							ss << line;
							ss >> _w;
							ss >> _h;
							if (_w*_h)
							{
								maze = new Maze(_w,_h);
								std::unique_ptr<ThisType> _m_(maze);
								_readData(maze,ifs,line);
								(*this) = std::move(*maze);
							}
							else{ return; }
						}
					}
					/*------------------------------------*/
				}
			}
		}
	}
	void _readData(ThisType * t, std::ifstream & ifs,std::string & line)
	{
		while (std::getline(ifs, line))
		{
			if (line.size())
			{
				//
				if (line=="column:")
				{
					for (u16 jj = 0; jj < t->getHeight();++jj)
					{
						std::getline(ifs, line);
						std::stringstream _ss_ ;
						_ss_ << line;
						for (u16 ii = 0; ii < t->getWidth();++ii)
						{
							_ss_>>(t->getColumnMatrix().get(ii,jj));
							t->getColumnMatrix().get(ii, jj) -= '0';
						}
					}
				}
				//
				if (line == "row:")
				{
					for (u16 jj = 0; jj < t->getHeight(); ++jj)
					{
						std::getline(ifs, line);
						std::stringstream _ss_;
						_ss_ << line;
						for (u16 ii = 0; ii < t->getWidth(); ++ii)
						{
							_ss_ >> (t->getRowMatrix().get(ii, jj));
							t->getRowMatrix().get(ii, jj) -= '0';
						}
					}
				}
				//
				if (line == "mask:")
				{
					for (u16 jj = 0; jj < t->getHeight(); ++jj)
					{
						std::getline(ifs, line);
						std::stringstream _ss_;
						_ss_ << line;
						for (u16 ii = 0; ii < t->getWidth(); ++ii)
						{
							_ss_ >> (t->getMaskMatrix().get(ii, jj));
							t->getMaskMatrix().get(ii, jj) -= '0';
						}
					}
				}
			}
		}
	}
};

#endif