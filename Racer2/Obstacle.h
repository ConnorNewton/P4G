#ifndef OBSTACLE_H
#define OBSTACLE_H

class obstacle {
public:
	obstacle();
	~obstacle();
private:
	DirectX::SimpleMath::Vector3 position;
	int rotSlot;
};

#endif