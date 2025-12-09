struct point : Windows::POINT
{
	friend constexpr std::strong_ordering operator<=>(point l,point r)noexcept = default;
	/*{
		if (l.x < r.x or (l.x == r.x and l.y < r.y))
			return std::strong_ordering::less;
		else if (l.x > r.x or (l.x == r.x and l.y > r.y))
			return std::strong_ordering::greater;
		else return std::strong_ordering::equivalent;
	}*/
	friend constexpr point operator+(point p)noexcept
	{
		return {+p.x,+p.y};
	}
	friend constexpr point operator-(point p)noexcept
	{
		return {-p.x,-p.y};
	}
	friend constexpr point operator+(point l,point r)noexcept
	{
		return {l.x+r.x,l.y+r.y};
	}
	friend constexpr point operator-(point l,point r)noexcept
	{
		return {l.x-r.x,l.y-r.y};
	}
	constexpr point& operator+=(point p)&noexcept
	{
		return this->x+=p.x,this->y+=p.y,*this;
	}
	constexpr point& operator-=(point p)&noexcept
	{
		return this->x-=p.x,this->y-=p.y,*this;
	}
};
std::ostream& operator<<(std::ostream& os, point p)
{
	return os << '(' << p.x << ',' << p.y << ')';
}