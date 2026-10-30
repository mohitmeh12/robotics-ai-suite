```{eval-rst}
.. meta::
   :description: Software references for deploying robot teleoperation and autonomous navigation.
   :keywords: AMR, deployment, teleoperation, wandering
```

# Deployable Robot Software Solutions

This software reference series showcases real-world deployable algorithms and pipelines from
the Robotics AI Suite.

You will learn how to:
- validate direct keyboard control of a deployed robot,
- map the robot's surroundings to provide real-world data for navigation, safety, and critical
  control systems,
- use the `wandering` pipeline, to put real-world sensing to practice, on a fully-featured
  robotics pipeline, ready to be deployed.

## Deploy Robot Teleop Using a Keyboard

[Read to the guide](teleop_deploy.md)

In this software reference, you will validate motor control on a deployed AAEON robot using a
keyboard before moving on to autonomous workloads.

## Deploying `wandering`

[Read to the guide](wandering_deploy.md)

In this software reference, you will deploy the `wandering` pipeline to demonstrate dynamic
sensing and control on a real-world robot. The pipeline maps and learns about the environment
using multiple sensing techniques.

## Next Steps

After completing the simulation and deployment software reference series, you are ready to use
the production-grade ingredients and pipelines included with the Robotics AI Suite in your own
software.

Check out the [Optimized Solutions](../../../components/optimized_solutions/index.md) to
explore AMR-specific ingredients, including components powered by OpenVINO™ and the
OpenVINO™ Physical AI framework, to add AI accelerated by Intel to your software stack.

:::{toctree}
:maxdepth: 1
:hidden:

teleop_deploy
wandering_deploy

:::

