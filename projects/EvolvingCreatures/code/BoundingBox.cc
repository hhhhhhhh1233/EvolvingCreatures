#include "BoundingBox.h"

static bool IsSorted(const std::vector<std::pair<int, float>>& Arr)
{
    for (int i = 0; i < Arr.size() - 1; i++)
    {
        if (Arr[i + 1].second < Arr[i].second)
            return false;
    }
    return true;
}

static void SortArr(std::vector<std::pair<int, float>>& Arr)
{
    while (!IsSorted(Arr))
    {
        for (int i = 0; i < Arr.size(); i++)
        {
            int j = i;
            while (j < Arr.size() - 1 && Arr[j].second > Arr[j + 1].second)
                j++;
            std::pair<int, float> Temp = Arr[j];
            Arr[j] = Arr[i];
            Arr[i] = Temp;
        }
    }
}

BoundingBox::BoundingBox(vec3 Position, vec3 Scale)
{
	Min = Position - Scale;
	Max = Position + Scale;
	/*std::cout << "Min: " << Min << "\n";*/
	/*std::cout << "Max: " << Max << "\n";*/
}

BoundingBox::BoundingBox()
{
	Min = vec3();
	Max = vec3();
}

bool BoundingBox::IsColliding(BoundingBox Other)
{
	std::vector<std::pair<int, float>> Sort;

	Sort = { {0, Min.x}, {0, Max.x}, {1, Other.Min.x}, {1, Other.Max.x} };
	SortArr(Sort);
	if (Sort[0].first == Sort[1].first)
		return false;

	Sort = { {0, Min.y}, {0, Max.y}, {1, Other.Min.y}, {1, Other.Max.y} };
	SortArr(Sort);
	if (Sort[0].first == Sort[1].first)
		return false;

	Sort = { {0, Min.z}, {0, Max.z}, {1, Other.Min.z}, {1, Other.Max.z} };
	SortArr(Sort);
	if (Sort[0].first == Sort[1].first)
		return false;

	return true;
}

bool BoundingBox::PointIsInShape(vec3 Point) const
{
	return (Min.x <= Point.x && Point.x <= Max.x) 
		&& (Min.y <= Point.y && Point.y <= Max.y) 
		&& (Min.z <= Point.z && Point.z <= Max.z);
}

vec3 BoundingBox::CalcDistanceToPoint(vec3 Point)
{
	vec3 Distance;

	/// Figure out x first
	if (Point.x <= Min.x)
		Distance.x = Min.x - Point.x;
	else if (Point.x >= Max.x)
		Distance.x = Max.x - Point.x;
	else
		Distance.x = 0;

	/// Then figure out y
	if (Point.y <= Min.y)
		Distance.y = Min.y - Point.y;
	else if (Point.y >= Max.y)
		Distance.y = Max.y - Point.y;
	else
		Distance.y = 0;

	/// Lastly z
	if (Point.z <= Min.z)
		Distance.z = Min.z - Point.z;
	else if (Point.z >= Max.z)
		Distance.z = Max.z - Point.z;
	else
		Distance.z = 0;

	return Distance;
}

void BoundingBox::Move(vec3 Diff)
{
	Min += Diff;
	Max += Diff;
}

vec3 BoundingBox::GetPosition() const
{
	return Min + ((Max - Min) * 0.5);
}

vec3 BoundingBox::GetScale() const
{
	return ((Max - Min) * 0.5);
}
