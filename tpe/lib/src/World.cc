/*
 * Copyright (C) 2020 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <string>
#include <memory>

#include <gz/common/Profiler.hh>

#include <gz/math/Pose3.hh>
#include "World.hh"
#include "Model.hh"
#include "Link.hh"

using namespace gz;
using namespace physics;
using namespace tpelib;

/////////////////////////////////////////////////
World::World() : Entity()
{
}

/////////////////////////////////////////////////
void World::SetTime(double _time)
{
  this->time = _time;
}

/////////////////////////////////////////////////
double World::GetTime() const
{
  return this->time;
}

/////////////////////////////////////////////////
void World::SetTimeStep(double _timeStep)
{
  this->timeStep = _timeStep;
}

/////////////////////////////////////////////////
double World::GetTimeStep() const
{
  return this->timeStep;
}

/////////////////////////////////////////////////
void World::Step()
{
  GZ_PROFILE("tpelib::World::Step");
  // apply updates to each model
  auto &children = this->GetChildren();
  for (auto it = children.begin(); it != children.end(); ++it)
  {
    auto model = std::dynamic_pointer_cast<Model>(it->second);
    model->UpdatePose(this->timeStep);
    auto &ents = model->GetChildren();
    for (auto linkIt = ents.begin(); linkIt != ents.end(); ++linkIt)
    {
      // if child of model is link
      auto link = std::dynamic_pointer_cast<Link>(linkIt->second);
      if (link)
      {
        link->UpdatePose(this->timeStep);
      }
    }
  }

  // check colliisions
  // the last bool arg tells the collision checker to return one single contact
  // point for each pair of collisions
  this->contacts = this->collisionDetector.CheckCollisions(children, true);

  for (auto it = children.begin(); it != children.end(); ++it)
    it->second->ResetPoseDirty();

  // increment world time by step size
  this->time += this->timeStep;
}

/////////////////////////////////////////////////
Entity &World::AddModel()
{
  std::size_t modelId = Entity::GetNextId();
  const auto[it, success] = this->GetChildren().insert(
    {modelId, std::make_shared<Model>(modelId)});
  return *it->second;
}

/////////////////////////////////////////////////
std::vector<Contact> World::GetContacts() const
{
  return this->contacts;
}
