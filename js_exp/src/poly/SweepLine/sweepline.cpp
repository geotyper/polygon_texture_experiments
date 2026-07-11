#include "sweepline.hpp"

#include <iostream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <array>
#include <vector>
#include <algorithm>


std::vector<sweepline::intersection_t> sweepline::find_intersections(
  const std::vector<geometry::segment_t> &line_segments,
  bool verbose,
  bool enable_color
) {

  return sweepline::solver(line_segments, verbose, enable_color).solve();
}

geometry::float_t sweepline::sweeplineX;

sweepline::solver::solver(const std::vector<geometry::segment_t> &line_segments, bool verbose, bool enable_color)
  : line_segments(line_segments), verbose(verbose) {


}

std::vector<sweepline::intersection_t> sweepline::solver::solve() {
  // initialize the sweepline to -inf
  sweepline::sweeplineX = -std::numeric_limits<geometry::float_t>::max();

  // initialize the event_queue by inserting the end points of the line segments
  // and populate vertical_segs with vertical segments
  init_event_queue();

  // sort vertical segments by x then y
  std::sort(vertical_segs.begin(), vertical_segs.end(),
    [](const geometry::segment_t &a, const geometry::segment_t &b) {
      return a.p.x == b.p.x? a.p.y < b.p.y : a.p.x < b.p.x;
    }
  );

  // find intersections between pairs of vertical line segments
  find_vertical_vertical_intersections();

  while(!event_queue.empty()) {
    sweepline::event_t top = *event_queue.begin();
    event_queue.erase(event_queue.begin());

    if(top.p.x < sweepline::sweeplineX) {
      continue;
    }

    // find intersections between a vertical line segment and one or more non-vertical segments
    find_vertical_nonvertical_intersections(top.p.x);

    // move sweepline to x coordinate of event being processed
    sweepline::sweeplineX = top.p.x;

    // get the active segments with an event at the point currently being processed
    // returns three arrays of active segment indices corresponding to event_t::type
    auto active_segs = get_active_segs(top);

    // remove all end points, insert all begin points and reorder the interior points
    update_segment_ordering(active_segs);

    // if no segments were newly inserted, the immediate left and right neighbours
    // of the deleted set of segments become adjacent candidates for intersection
    if(active_segs[sweepline::event_t::type::begin].size()
        + active_segs[sweepline::event_t::type::interior].size() == 0)
            handle_no_newly_inserted(top.p);
    else
      handle_extremes_of_newly_inserted();
    // else the left and right extremes among the set of newly inserted segments
    // must be checked for intersection with the immediate left and right neighbours respectively

    // reset the sweepline to the x coordinate of event being processed
    sweepline::sweeplineX = top.p.x;

    // finally report the union of all active segments as an intersection if there are two or more of them
    if(active_segs[0].size() + active_segs[1].size() + active_segs[2].size() > 1)
      report_intersection(top.p, std::move(active_segs));

  }

  merge_intersection_points();

  return result;
}

void sweepline::solver::init_event_queue() {


  for(size_t i = 0; i < line_segments.size(); i++) {
    const auto &p = line_segments[i].p;
    const auto &q = line_segments[i].q;

    if(std::fabs(p.x - q.x) < geometry::EPS) {
      // handle (vertical) segments with same slope as sweepline separately
      vertical_segs.emplace_back(line_segments[i]);
    } else {
      // insert the begin and end points of each segment in the event queue
      event_queue.insert({ p, sweepline::event_t::type::begin, i});
      event_queue.insert({ q, sweepline::event_t::type::end,   i});
    }
  }

}

void sweepline::solver::find_vertical_vertical_intersections() {
  for(size_t i = 0; i + 1 < vertical_segs.size(); i++) {
    if(vertical_segs[i].q == vertical_segs[i + 1].p) {
      sweepline::intersection_t it {
        vertical_segs[i].q,
        std::vector<size_t> {
          vertical_segs[i].seg_id,
          vertical_segs[i + 1].seg_id
        }
      };


      result.emplace_back(it);
    }
  }
}

void sweepline::solver::find_vertical_nonvertical_intersections(geometry::float_t max_vsegx) {
  while(vert_idx < vertical_segs.size()
    and vertical_segs[vert_idx].p.x < sweepline::sweeplineX - geometry::EPS)
      vert_idx++;

  while(vert_idx < vertical_segs.size()
    and vertical_segs[vert_idx].p.x <= max_vsegx + geometry::EPS) {

      auto &vseg = vertical_segs[vert_idx];
      sweepline::sweeplineX = vseg.p.x;

      auto itr = seg_ordering.lower_bound(geometry::segment_t{ vseg.p, vseg.p, 0 });

      while(itr != seg_ordering.end()) {
        geometry::float_t it_y = itr->eval_y(sweepline::sweeplineX);

        if(it_y > vseg.q.y + geometry::EPS)
          break;

        sweepline::intersection_t it {
          geometry::point_t{ sweepline::sweeplineX, it_y },
          std::vector<size_t>{ itr->seg_id, vseg.seg_id }
        };


        result.emplace_back(it);

        ++itr;
      }

      vert_idx++;
  }
}

std::array<std::vector<size_t>, 3> sweepline::solver::get_active_segs(sweepline::event_t top) {
  // array of all segments with an event at the point currently being processed
  //   active[event_t::type::begin]    -> list of segments which begin at this point
  //   active[event_t::type::interior] -> list of segments which intersect with some other segment at this point
  //   active[event_t::type::end]      -> list of segments which end at this point
  std::array<std::vector<size_t>, 3> active;
  active[top.tp].push_back(top.seg_id);

  // get all segments with an event at top.p and add them to one of the above
  while(!event_queue.empty() and event_queue.begin()->p == top.p) {
    sweepline::event_t nxt_top = *event_queue.begin();
    event_queue.erase(event_queue.begin());
    active[nxt_top.tp].push_back(nxt_top.seg_id);
  }


  return active;
}

void sweepline::solver::update_segment_ordering(const std::array<std::vector<size_t>, 3> &active) {
  // remove all end event segments
  for(size_t idx: active[sweepline::event_t::type::end])
    seg_ordering.erase(line_segments[idx]);

  // remove all interior event segments
  for(size_t idx: active[sweepline::event_t::type::interior])
    seg_ordering.erase(line_segments[idx]);

  // increment the sweepline by a very small amount, just past the intersection point
  sweepline::sweeplineX += geometry::EPS_INC;

  max_y = -std::numeric_limits<geometry::float_t>::max();
  min_y = std::numeric_limits<geometry::float_t>::max();

  // insert all begin type events
  for(int idx: active[sweepline::event_t::type::begin]) {
    min_y = std::min(min_y, line_segments[idx].eval_y(sweepline::sweeplineX));
    max_y = std::max(max_y, line_segments[idx].eval_y(sweepline::sweeplineX));
    seg_ordering.insert(line_segments[idx]);
  }

  // re-insert all interior type events (so that ordering is updated)
  for(int idx: active[sweepline::event_t::type::interior]) {
    min_y = std::min(min_y, line_segments[idx].eval_y(sweepline::sweeplineX));
    max_y = std::max(max_y, line_segments[idx].eval_y(sweepline::sweeplineX));
    seg_ordering.insert(line_segments[idx]);
  }
}

void sweepline::solver::handle_no_newly_inserted(geometry::point_t cur) {
  auto b_right = seg_ordering.lower_bound(geometry::segment_t{ cur, cur, 0 });
  if(b_right != seg_ordering.end() and b_right != seg_ordering.begin()) {
    auto b_left = b_right;
    --b_left;
    if(geometry::is_intersecting(*b_left, *b_right)) {
      geometry::point_t pt = geometry::intersection_point(*b_left, *b_right);
      sweepline::event_t::type tp1 = b_left->p == pt? sweepline::event_t::type::begin : sweepline::event_t::type::interior;
      sweepline::event_t::type tp2 = b_right->p == pt? sweepline::event_t::type::begin : sweepline::event_t::type::interior;

      event_queue.insert(sweepline::event_t{ pt, tp1, b_left->seg_id  });
      event_queue.insert(sweepline::event_t{ pt, tp2, b_right->seg_id });
    }
  }
}

void sweepline::solver::handle_extremes_of_newly_inserted() {
  geometry::point_t left { sweepline::sweeplineX, min_y - 2 * geometry::EPS }, right { sweepline::sweeplineX, max_y + 2 * geometry::EPS };
  auto b_right = seg_ordering.lower_bound(geometry::segment_t{ right, right, 0 });
  auto s_left  = seg_ordering.lower_bound(geometry::segment_t{ left,  left,  0 });

  // check for candidate intersection at the right extreme
  if(b_right != seg_ordering.end() and b_right != seg_ordering.begin()) {
    auto s_right = b_right;
    --s_right;

    if(is_intersecting(*s_right, *b_right)) {
      geometry::point_t pt = intersection_point(*s_right, *b_right);
      sweepline::event_t::type tp1 = s_right->p == pt? sweepline::event_t::type::begin : sweepline::event_t::type::interior;
      sweepline::event_t::type tp2 = b_right->p == pt? sweepline::event_t::type::begin : sweepline::event_t::type::interior;

      event_queue.insert(sweepline::event_t{ pt, tp1, s_right->seg_id });
      event_queue.insert(sweepline::event_t{ pt, tp2, b_right->seg_id });
    }
  }

  // check for candidate intersection at the left extreme
  if(s_left != seg_ordering.end() and s_left != seg_ordering.begin()) {
    auto b_left = s_left;
    --b_left;

    if(is_intersecting(*b_left, *s_left)) {
      geometry::point_t pt = intersection_point(*b_left, *s_left);
      sweepline::event_t::type tp1 = b_left->p == pt? sweepline::event_t::type::begin : sweepline::event_t::type::interior;
      sweepline::event_t::type tp2 = s_left->p == pt? sweepline::event_t::type::begin : sweepline::event_t::type::interior;

      event_queue.insert(sweepline::event_t{ pt, tp1, b_left->seg_id });
      event_queue.insert(sweepline::event_t{ pt, tp2, s_left->seg_id });
    }
  }
}

void sweepline::solver::report_intersection(geometry::point_t cur, std::array<std::vector<size_t>, 3> &&active) {
  active[0].insert(active[0].end(), active[1].begin(), active[1].end());
  active[0].insert(active[0].end(), active[2].begin(), active[2].end());

  result.emplace_back(sweepline::intersection_t{ cur, std::move(active[0]) });

}

void sweepline::solver::merge_intersection_points() {
  std::sort(result.begin(), result.end(),
    [](const sweepline::intersection_t &a, const sweepline::intersection_t &b) {
      return a.pt.x == b.pt.x? a.pt.y < b.pt.y : a.pt.x < b.pt.x;
    }
  );

  std::vector<sweepline::intersection_t> merged;
  for(size_t i = 0, j; i < result.size(); i = j) {
    std::vector<size_t> indices;
    for(j = i; j < result.size() and result[j].pt == result[i].pt; j++)
      indices.insert(indices.end(), result[j].segments.begin(), result[j].segments.end());

    std::sort(indices.begin(), indices.end());
    indices.erase(std::unique(indices.begin(), indices.end()), indices.end());

    merged.emplace_back(
      sweepline::intersection_t {
        result[i].pt, indices
      }
    );
  }

  result = std::move(merged);
}
