#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "tile.h"
#include "project.h"

using namespace std;


map<uint32_t, string> Tile::m_collisionChannelNames;


Tile::Tile(uint16_t width, uint16_t height, uint16_t depth, uint16_t frames)
{
	m_width = width;
	m_height = height;
	m_depth = depth;
	m_frames = frames;
	m_pitch = (((size_t)m_width * (size_t)m_depth) + 7) / 8;
	m_frameSize = m_pitch * (size_t)height;
	m_size = m_frameSize * (size_t)m_frames;
	m_data = new uint8_t[m_size];
	memset(m_data, 0, m_size);
}


Tile::Tile(const Tile& other)
{
	m_width = other.m_width;
	m_height = other.m_height;
	m_depth = other.m_depth;
	m_frames = other.m_frames;
	m_pitch = other.m_pitch;
	m_frameSize = other.m_frameSize;
	m_size = other.m_size;
	m_palette = other.m_palette;
	m_paletteOffset = other.m_paletteOffset;
	m_data = new uint8_t[m_size];
	memcpy(m_data, other.m_data, m_size);
}


Tile::~Tile()
{
	delete[] m_data;
}


uint8_t* Tile::GetData(uint16_t frame)
{
	if (frame >= m_frames)
		frame = m_frames - 1;
	return &m_data[(size_t)frame * m_frameSize];
}


const uint8_t* Tile::GetData(uint16_t frame) const
{
	if (frame >= m_frames)
		frame = m_frames - 1;
	return &m_data[(size_t)frame * m_frameSize];
}


void Tile::SetFrameCount(uint16_t frames)
{
	uint8_t* newData = new uint8_t[(size_t)frames * m_frameSize];
	memset(newData, 0, (size_t)frames * m_frameSize);

	uint16_t copyFrames = frames;
	if (copyFrames > m_frames)
		copyFrames = m_frames;
	memcpy(newData, m_data, (size_t)copyFrames * m_frameSize);

	delete[] m_data;
	m_data = newData;
	m_size = (size_t)frames * m_frameSize;
	m_frames = frames;
}


void Tile::CopyFrame(uint16_t from, uint16_t to)
{
	if (from >= m_frames)
		from = m_frames - 1;
	if (to >= m_frames)
		to = m_frames - 1;
	if (from == to)
		return;

	memcpy(&m_data[(size_t)to * m_frameSize], &m_data[(size_t)from * m_frameSize], m_frameSize);
}


void Tile::SwapFrames(uint16_t from, uint16_t to)
{
	if (from >= m_frames)
		from = m_frames - 1;
	if (to >= m_frames)
		to = m_frames - 1;
	if (from == to)
		return;

	uint8_t* tempData = new uint8_t[m_frameSize];
	memcpy(tempData, &m_data[(size_t)to * m_frameSize], m_frameSize);
	memcpy(&m_data[(size_t)to * m_frameSize], &m_data[(size_t)from * m_frameSize], m_frameSize);
	memcpy(&m_data[(size_t)from * m_frameSize], tempData, m_frameSize);
	delete[] tempData;
}


void Tile::DuplicateFrame(uint16_t frame)
{
	if (frame >= m_frames)
		frame = m_frames - 1;

	size_t trailingFrames = m_frames - frame;
	SetFrameCount(m_frames + 1);
	memmove(&m_data[((size_t)frame + 1) * m_frameSize], &m_data[(size_t)frame * m_frameSize], m_frameSize * trailingFrames);
	memcpy(&m_data[(size_t)frame * m_frameSize], &m_data[((size_t)frame + 1) * m_frameSize], m_frameSize);
}


void Tile::RemoveFrame(uint16_t frame)
{
	if ((frame >= m_frames) || (m_frames == 1))
		return;
	size_t trailingFrames = (m_frames - frame) - 1;
	memmove(&m_data[(size_t)frame * m_frameSize], &m_data[((size_t)frame + 1) * m_frameSize], m_frameSize * trailingFrames);
	SetFrameCount(m_frames - 1);
}


shared_ptr<Tile> Tile::GetTileForSingleFrame(uint16_t frame)
{
	if (frame >= m_frames)
		frame = m_frames - 1;
	shared_ptr<Tile> result = make_shared<Tile>(m_width, m_height, m_depth);
	result->m_palette = m_palette;
	result->m_paletteOffset = m_paletteOffset;
	memcpy(result->m_data, &m_data[(size_t)frame * m_frameSize], m_frameSize);
	return result;
}


void Tile::InsertFrameFromTile(uint16_t frame, const shared_ptr<Tile>& tile)
{
	if (m_frameSize != tile->m_frameSize)
		return;
	if (frame > m_frames)
		frame = m_frames;

	size_t trailingFrames = m_frames - frame;
	SetFrameCount(m_frames + 1);
	memmove(&m_data[((size_t)frame + 1) * m_frameSize], &m_data[(size_t)frame * m_frameSize], m_frameSize * trailingFrames);
	memcpy(&m_data[(size_t)frame * m_frameSize], tile->m_data, m_frameSize);
}


void Tile::SetPalette(const shared_ptr<Palette> palette, uint8_t offset)
{
	m_palette = palette;
	m_paletteOffset = offset;
}


vector<BoundingRect> Tile::GetCollision(uint32_t channel) const
{
	if (channel == COLLISION_CHANNEL_ALL)
		return m_collision;
	auto i = m_collisionChannels.find(channel);
	if (i == m_collisionChannels.end())
		return vector<BoundingRect>();
	return i->second;
}


void Tile::SetCollision(uint32_t channel, const vector<BoundingRect>& collision)
{
	if (channel == COLLISION_CHANNEL_ALL)
	{
		m_collision = collision;
		return;
	}

	if (collision.size() == 0)
		m_collisionChannels.erase(channel);
	else
		m_collisionChannels[channel] = collision;
}


Json::Value Tile::Serialize()
{
	Json::Value tile(Json::objectValue);
	if (m_palette)
	{
		tile["palette"] = m_palette->GetId();
		tile["offset"] = m_paletteOffset;
	}

	string data;
	for (size_t i = 0; i < m_size; i++)
	{
		char byteStr[3];
		sprintf(byteStr, "%.2x", m_data[i]);
		data += byteStr;
	}
	tile["data"] = data;

	if (m_collision.size() > 0)
	{
		Json::Value collision(Json::arrayValue);
		for (auto& i : m_collision)
		{
			Json::Value rect(Json::objectValue);
			rect["x"] = i.x;
			rect["y"] = i.y;
			rect["w"] = i.width;
			rect["h"] = i.height;
			collision.append(rect);
		}
		tile["collision"] = collision;
	}

	if (m_collisionChannels.size() > 0)
	{
		Json::Value collisionChannels(Json::arrayValue);
		for (auto& i : m_collisionChannels)
		{
			Json::Value collision(Json::objectValue);
			collision["channel"] = i.first;
			Json::Value bounds(Json::arrayValue);
			for (auto& j : i.second)
			{
				Json::Value rect(Json::objectValue);
				rect["x"] = j.x;
				rect["y"] = j.y;
				rect["w"] = j.width;
				rect["h"] = j.height;
				bounds.append(rect);
			}
			collision["bounds"] = bounds;
			collisionChannels.append(collision);
		}
		tile["collision_channels"] = collisionChannels;
	}

	return tile;
}


shared_ptr<Tile> Tile::Deserialize(shared_ptr<Project> project, const Json::Value& data,
	size_t width, size_t height, size_t depth, size_t frames)
{
	shared_ptr<Tile> result = make_shared<Tile>(width, height, depth, frames);
	if (data.isMember("palette"))
	{
		result->m_palette = project->GetPaletteById(data["palette"].asString());
		result->m_paletteOffset = (uint8_t)data["offset"].asUInt();
	}

	string dataStr = data["data"].asString();
	if (dataStr.size() != (result->m_size * 2))
		return shared_ptr<Tile>();

	for (size_t i = 0; i < result->m_size; i++)
		result->m_data[i] = (uint8_t)strtoul(dataStr.substr(i * 2, 2).c_str(), nullptr, 16);

	if (data.isMember("collision"))
	{
		for (auto& i : data["collision"])
		{
			BoundingRect rect;
			rect.x = (uint16_t)i["x"].asUInt();
			rect.y = (uint16_t)i["y"].asUInt();
			rect.width = (uint16_t)i["w"].asUInt();
			rect.height = (uint16_t)i["h"].asUInt();
			result->m_collision.push_back(rect);
		}
	}

	if (data.isMember("collision_channels"))
	{
		for (auto& i : data["collision_channels"])
		{
			uint32_t idx = i["channel"].asUInt();
			vector<BoundingRect> bounds;
			for (auto& j : i["bounds"])
			{
				BoundingRect rect;
				rect.x = (uint16_t)j["x"].asUInt();
				rect.y = (uint16_t)j["y"].asUInt();
				rect.width = (uint16_t)j["w"].asUInt();
				rect.height = (uint16_t)j["h"].asUInt();
				bounds.push_back(rect);
			}
			result->m_collisionChannels[idx] = bounds;
		}
	}

	return result;
}


void Tile::RegisterCollisionChannel(uint32_t idx, const string& name)
{
	m_collisionChannelNames[idx] = name;
}


map<uint32_t, string> Tile::GetCollisionChannelNames()
{
	return m_collisionChannelNames;
}
