
#ifndef __GENERATE_Maze__H__0_
#define __GENERATE_Maze__H__0_ 1

#ifndef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS 1
#endif

#include<cstdint>
#include<cstddef>
#include<stack>
#include<utility>
#include<cstdlib>
#include"Maze.h"
#include<vector>
#include<algorithm>
#include<ctime>
#include<cstdlib>

class GenerateMaze
{
public:
	std::uint16_t width;
	std::uint16_t height;

	typedef std::pair < std::uint16_t, std::uint16_t > pos_t;
	typedef std::stack<pos_t> stack_t;

	GenerateMaze(std::uint16_t i_width=100, std::uint16_t i_height=100):width(i_width),
	height(i_height)
	{
		time_t _timer_ = (rand()+rand()+rand())%125;
		std::srand(int(std::time(&_timer_)));
	}

	Maze generate()
	{
		Maze maze(width,height);
		stack_ = stack_t();
		stack_.push(pos_t{0,0});
		auto & mask = maze.getMaskMatrix();
		auto & row = maze.getRowMatrix();
		auto & column = maze.getColumnMatrix();

		pos_t nei_pos;//neighbour pos
		pos_t cur_pos;//current pos

		while (true!=_isAllVisited(maze))
		{
			if (stack_.size())
			{//stack_.size
				cur_pos = stack_.top();
				mask.get(cur_pos.first,cur_pos.second)=true;
				if (_getNeighbourPosRand(cur_pos, nei_pos, mask))
				{
					stack_.push(nei_pos);
					if (nei_pos.first==cur_pos.first)
					{
						row.get(nei_pos.first,std::min(nei_pos.second, cur_pos.second))=false;
					}
					else
					{
						column.get(std::min(nei_pos.first,cur_pos.first),cur_pos.second)=false;
					}
				}
				else
				{
					/*随机回溯*/
					if (stack_.size() < 3)
					{
						stack_.pop();
					}
					else
					{
						std::uint16_t _ii_ = 1;
						do
						{
							_ii_ = std::uint16_t(stack_.size() * 2 / 3);
							_ii_ = rand() % _ii_;
						} while (!(_ii_&&(_ii_<stack_.size())));
						for (;_ii_!=0;--_ii_)
						{
							stack_.pop();
						}
					}
				}
			}//stack.size
			else
			{
				/*随机重新开始*/
				std::vector<pos_t> _vp_;
				for (u16 ii = 0; ii < width;++ii)
				{
					for (u16 jj = 0; jj < height;++jj)
					{
						if (mask.get(ii, jj) && _getNeighbourPosRand({ii,jj}, nei_pos, mask))
						{
							_vp_.push_back({ ii, jj });
						}
					}
				}
				if (_vp_.size())
				stack_.push(_vp_[rand()%(_vp_.size())]);
			}
		}
		return std::move(maze);
	}

private:
	typedef std::uint16_t u16;
	stack_t stack_;
	bool _isAllVisited(const Maze & _maze)
	{
		const auto & m_ = _maze.getMaskMatrix();
		for (u16 ii = 0; ii < m_.width;++ii)
		{
			for (u16 jj = 0; jj < m_.height;++jj)
			{
				if (false==m_.get(ii,jj))
				{
					return false;
				}
			}
		}
		return true;
	}
	bool _getNeighbourPosRand(const pos_t & i_pos,pos_t & o_pos,const Maze::Matrix2<unsigned char> & i_mask)
	{
		std::vector<pos_t> _vp_;
		pos_t _p_ = {i_pos.first+1,i_pos.second};
		if (_p_.first < width)
		{
			if (i_mask.get(_p_.first,_p_.second)==false)
			_vp_.push_back(_p_); 
		}
		if (_p_.first > 1)
		{ 
			_p_.first -= 2; 
			if (i_mask.get(_p_.first, _p_.second)==false)
			_vp_.push_back(_p_); 
		}
		_p_ = { i_pos.first, i_pos.second + 1 };
		if (_p_.second<height)
		{ 
			if (i_mask.get(_p_.first, _p_.second)==false)
			_vp_.push_back(_p_); 
		}
		if (_p_.second > 1) 
		{ 
			_p_.second -= 2; 
			if (i_mask.get(_p_.first, _p_.second)==false)
			_vp_.push_back(_p_); 
		}
		if (_vp_.size())
		{
			if (_vp_.size()==1)
			{
				o_pos = _vp_[0];
			}
			else
			{
				o_pos = _vp_[(rand()%(_vp_.size()))];
			}
			return true;
		}
		return false;
	}

	
};

#endif