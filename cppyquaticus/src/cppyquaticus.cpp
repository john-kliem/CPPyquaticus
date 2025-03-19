#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdlib>


 /**
  * @brief This is an implementation of pyquaticus's pettingzoo environment written in c++ to c
  * 
  * 
  */
class CTFpyquaticus {
public:
    std::vector<std::string> agents;
    std::vector<std::string> obstacles;
    std::vector<std::string> flags;
    bool done;
    /**
      * @brief Initializes a capture the flag game along with agents.
      * @param num_teams number of teams in the game currently only supports two teams
      * @param num_agents number of agents per team currently in the game.
      * @param num_possible number of agents possible on each team.
      * @param config game configuration.
      * @return Returns a maritime capture the flag env object.
    */
   CTFpyquaticus() {
        for (int i = 0; i < numTeams; ++i) {
            std::cout << "Processing agent_" << i + 1 << std::endl;
        }
        agents = {"agent_1", "agent_2", "agent_3"};
        reset();
    }

    void reset() {
        agent_states.clear();
        for (const auto& agent : agents) {
            agent_states[agent] = 0;  // Initial state
        }
        done = false;
        std::cout << "Environment reset!" << std::endl;
    }

    void step(const std::unordered_map<std::string, int>& actions) {
        if (done) {
            std::cout << "Environment is already finished. Reset to continue." << std::endl;
            return;
        }

        for (const auto& [agent, action] : actions) {
            if (agent_states.find(agent) != agent_states.end()) {
                agent_states[agent] += action;  // Apply action
                std::cout << agent << " took action " << action << ", new state: " << agent_states[agent] << std::endl;
            }
        }

        // Simple condition to finish after some steps
        if (rand() % 10 > 7) {  
            done = true;
            std::cout << "Environment finished!" << std::endl;
        }
    }

    void render() {
        std::cout << "Rendering environment state:" << std::endl;
        for (const auto& [agent, state] : agent_states) {
            std::cout << agent << " is at state " << state << std::endl;
        }
    }

    void close() {
        std::cout << "Environment closed." << std::endl;
    }
};

int main() {
    CTFpyquaticus env;
    env.render();

    for (int i = 0; i < 5; ++i) {
        std::unordered_map<std::string, int> actions = {
            {"agent_1", 1}, {"agent_2", -1}, {"agent_3", 2}
        };
        env.step(actions);
        env.render();
        if (env.done) break;
    }

    env.close();
    return 0;
}